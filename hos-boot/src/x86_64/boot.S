.code32
# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot
.set PML4_BASE, 0x1000

# Declare a header as in the Multiboot Standard. We put this into a special
# section so we can force the header to be in the start of the final program.
# You don't need to understand all these details as it is just magic values that
# is documented in the multiboot standard. The bootloader will search for this
# magic sequence and recognize us as a multiboot kernel.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Currently the stack pointer register (esp) points at anything and using it may
# cause massive harm. Instead, we'll provide our own stack. We will allocate
# room for a small temporary stack by creating a symbol at the bottom of it,
# then allocating 16384 bytes for it, and finally creating a symbol at the top.
.section .bootstrap_stack
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded. It
# doesn't make sense to return from this function as the bootloader is gone.
.section .text
.global _start
.extern initMultiboot64
.type _start, @function
_start:
	movl $stack_top, %esp
        push %ebx               # The multiboot header is in ebx, this let's the C code use it
        call initMultiboot64
        hlt

.global _enterLongMode
.type _enterLongMode, @function
_enterLongMode:
        # edi should be set to PML4_BASE
        # ebx should be set to multiboot info
        # edx should be set to kernel entry point

        # We now seek to enter long mode before calling C
        # We do this by... (Chapter 14, AMD64 Developer's Manual, Volume 2)
        #    - Enabling the 64-bit page translation table entries, by setting CR4.PAE
        #    - Setting the Long-mode page tables (set CR3 to PML4)
        #    - Enabling long mode
        #    - Enable paging to activate long mode
        #    - At this point, we're still in compatibility mode, due to our gdt, we will want to change gdts, and then do a long jump into long mode

        # Step 1: Enable 64-bit page translation table

        mov %cr4, %eax
        bts $5, %eax # Bit 5 is PAE, Physical-Address Extension
        mov %eax, %cr4

        # Step 2: Set up long-mode page tables
        mov %edi, %cr3

        # Step 3: Enable long mode

        mov $0xc0000080, %ecx
        rdmsr
        or $(1 << 8), %eax
        wrmsr

        # Step 4: Enable paging by setting CR0.PG

        mov %cr0, %eax
        bts $31, %eax
        mov %eax, %cr0

        # Step 5: Load 64-bit gdt, and do far jump

        mov $gdt_pointer, %eax
        lgdt (%eax)

        ljmp $0x08, $Realm64

.code64

.extern kernel_entry
Realm64:
        cli
        mov $0x10, %ax
        mov %ax, %ds
        mov %ax, %es
        mov %ax, %fs
        mov %ax, %gs

        /* We have to resort to this trickery to make relocations work right */
        //mov $0, %rbx
        //movl $kernel_entry, %ebx /* Move kernel_entry into ebx */
        //movq (%rbx), %rax /* Copy contents of kernel_entry into eax */
        movabs $0xffff800000000001, %rax /* Jump to entry point, and say hello to kernel land! */
        jmp *%rax
        hlt

        .code32
        .section .data
gdt:
        # Null descriptor
        .short 0                    # Limit (low)
        .short 0                    # Base (low)
        .byte 0                    # Base (middle)
        .byte 0                    # Access
        .byte 0                    # Granularity
        .byte 0                    # Base (high)
        ## Code descriptor
        .short 0                    # Limit (low)
        .short 0                    # Base (low)
        .byte 0                    # Base (middle)
        .byte 0b10011000           # Access
        .byte 0b00100000           # Granularity
        .byte 0                    # Base (high)
        ## Data descriptor
        .short 0                    # Limit (low)
        .short 0                    # Base (low)
        .byte 0                    # Base (middle)
        .byte 0b10010000           # Access
        .byte 0b00000000           # Granularity
        .byte 0                    # Base (high)
gdt_pointer: # We use .long here instead of .quad to let gas compile this as 64 bit without emitting 64-bit relocations
        # which cannot be handled in a 32-bit elf
        .short . - gdt - 1
        .long  gdt
        .long 0