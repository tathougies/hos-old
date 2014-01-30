.code64

# Currently the stack pointer register (esp) points at anything and using it may
# cause massive harm. Instead, we'll provide our own stack. We will allocate
# room for a small temporary stack by creating a symbol at the bottom of it,
# then allocating 16384 bytes for it, and finally creating a symbol at the top.
.section .bootstrap_stack
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
        movl $0xB8000, %edi
        movq $0x1F201F201F201F20, %rax
        movl $500, %ecx
        rep stosq
        movq $0x1F431F421F411F40, %rax
        movq %rax, 0xb8000
        movabs $stack_top, %rsp
        push %rbx
        call initRawRts
        hlt