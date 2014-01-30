/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */

#include <common/options.h>
#include <common/common.h>
#include <common/string.h>
#include <common/elf64.h>

#include <i386/terminal.h>
#include <i386/multiboot.h>

#define ONE_GIGABYTE           (1024 * 1024 * 1024)
#define TWO_MEGABYTES          (2 * 1024 * 1024)
#define PAGE_SIZE              0x1000
#define ENTRIES_PER_PAGE_TABLE 512
#define PAGE_ACCESS_WRITABLE   0x2
#define PAGE_ACCESS_PRESENT    0x1
#define PAGE_ACCESS_BITS       0x3
#define KERNEL_LOAD_ADDRESS    0xffff800000000000
#define PML4_ENTRY             256

uint64_t kernel_entry = 0; /* The assembler reads from here */

typedef struct {
  char *kernel_name;
  uint32_t kernel_start, kernel_end;

  uint32_t page_table_start, page_table_end;

  uint32_t highest_module_address;

  uint32_t bss_pages;

  uint32_t pml4_ptr;
} boot_options_t;

boot_options_t global_options = {
  /* Kernel information */
  "Interlude", /* Default kernel name */
  0, 0,        /* Kernel location (filled in parse_modules) */

  /* Page table info */
  0, 0,

  0, /* highest module address */
  0, /* bss pages */
  0  /* PML 4 ptr */
};

static void error(const char *msg)
{
  set_fg_color(LIGHT_RED);
  puts(msg);
  for (;;);
}

static int kernel_option(char *name, char *value, void *data)
{
  boot_options_t *o = (boot_options_t *)data;

  if (!value) {
    set_error("No value given for kernel option");
    return 0;
  }

  o->kernel_name = value;
  return 1;
}

static void parse_cmdline(multiboot_info_t *multiboot)
{
  static const option_t options[] = {
    {"kernel", kernel_option},
    {NULL, NULL}
  };
  int result;

  printf("Using options: %s\n", multiboot->command_line);

  result = parse_command_line(multiboot->command_line, options, (void *)&global_options);
  if (!result)
    error(get_error());
}

static void check_multiboot(multiboot_info_t *multiboot)
{
  if (!(multiboot->flags & MULTIBOOT_MODULES_FLAG) || multiboot->mods_count == 0)
    error("Bootstrapper loaded with no kernel module");

  if (!(multiboot->flags & MULTIBOOT_MEM_MAP_FLAG))
    error("Cannot find memory map");
}

/* This function searches for the kernel module. It's either 'Interlude' or the name given
   in the kernel= command line option */
static void parse_modules(multiboot_info_t *multiboot)
{
  int i = 0;
#ifdef DEBUG
  printf("\nLoaded modules:\n");
  for (i = 0;i < multiboot->mods_count;++i) {
    multiboot_module_t *mod = multiboot->mods + i;
    printf("    - %s: 0x%x -- 0x%x\n", mod->mod_name, mod->mod_start, mod->mod_end);
  }
#endif
  printf("Going to load %s as kernel...\n", global_options.kernel_name);
  for (i = 0;i < multiboot->mods_count;++i) {
    multiboot_module_t *mod = multiboot->mods + i;
    char *mod_name, *mod_args, *saveptr;
    mod_name = strtok_r(mod->mod_name, " ", &saveptr);
    mod_args = saveptr; /* saveptr will point to the remaining strings */

    if (strcmp(mod_name, global_options.kernel_name) == 0) {
      global_options.kernel_start = mod->mod_start;
      global_options.kernel_end = ALIGN_4K_UP(mod->mod_end);
    }

    if (ALIGN_4K_UP(mod->mod_end) > global_options.highest_module_address)
      global_options.highest_module_address = ALIGN_4K_UP(mod->mod_end);

    if (mod_args)
      /* If the module has arguments, then we need to restore the module string to what it was
       * before by adding a space after mod_name to replace the NUL terminator. */
      *(mod_args - 1) = ' ';
  }
}

static int estimate_pages_needed_for_kernel()
{
  /* We need to read in the ELF section table to figure out how big all the sections are
   * together. For debug builds, we also print out the section table as we're figuring it all out */

  Elf64_Ehdr *hdr = (Elf64_Ehdr *) global_options.kernel_start;
  Elf64_Shdr *string_table_section_hdr = (Elf64_Shdr *) (uint32_t)
    (global_options.kernel_start + hdr->e_shoff + hdr->e_shstrndx * hdr->e_shentsize);
  uint32_t string_table_offs = global_options.kernel_start + string_table_section_hdr->sh_offset;

  uint32_t kernel_size = 0, bss_size = 0;

  int i;

#if DEBUG
  printf("Kernel sections:\n");
#endif

  /* We ignore the first entry, because it should be 0. */
  /* We parse all sections, but we only care about .bss, .text, .data, and .rodata */
  for (i = 1; i < hdr->e_shnum; ++i) {
    Elf64_Shdr *section = (Elf64_Shdr *) (uint32_t) (global_options.kernel_start + hdr->e_shoff + i * hdr->e_shentsize);
    char *section_name = (char *) (string_table_offs + section->sh_name);

    int is_bss =
      /* .bss */
      ((section->sh_type == SHT_NOBITS) && (section->sh_flags & SHF_ALLOC) &&
       (section->sh_flags & SHF_WRITE));
    int will_load =
      is_bss ||
      /* .data */
      ((section->sh_type == SHT_PROGBITS) && (section->sh_flags & SHF_ALLOC) &&
       (section->sh_flags & SHF_WRITE)) ||
      /* .rodata */
      ((section->sh_type == SHT_PROGBITS) && (section->sh_flags & SHF_ALLOC)) ||
      /* .text */
      ((section->sh_type == SHT_PROGBITS) && (section->sh_flags & SHF_ALLOC) &&
       (section->sh_flags && SHF_EXECINSTR));

#if DEBUG
    printf("   - (%s) %s -- Offset 0x%lX, VAddr 0x%lX, Size 0x%lX\n",
           will_load ? "WILL LOAD" : "NO LOAD",
           section_name,
           section->sh_offset,
           section->sh_addr,
           section->sh_size);
#endif
    if (will_load && (!ALIGNED_4K(section->sh_addr) || !ALIGNED_4K(section->sh_offset)))
      error("Can't load unaligned section");

    kernel_size += ALIGN_4K_UP(section->sh_size);

    /* If we're looking at a bss section, we will also have to reserve pages for the uninitialized
     * data */
    if (is_bss)
      bss_size += ALIGN_4K_UP(section->sh_size);
  }

#if DEBUG
  printf("The kernel has a loaded size of 0x%X\n", kernel_size);
#endif

  /* Two cases.
   *
   * 1) The kernel is > 2MB and < 1GB. We will need at most 512 page tables, plus 3 to fill in the
   *    paging hierarchy (PML4, PDPT, PDT).
   * 2) The kernel is > 1GB. First of all, WTF? A 1GB kernel? This is an error.
   */

  if (!ALIGNED_4K(global_options.kernel_start))
    error("Cannot map non-page-aligned kernel");

  if (kernel_size > ONE_GIGABYTE) /* Case 2 */
    error("Why the fuck do you have a one gigabyte kernel??");

  /* Now we take the total size of the kernel, in pages */
  global_options.bss_pages = bss_size / PAGE_SIZE;

  /* We will need one PML4.
   * We need 2 PDPTs: one for the kernel, one for us.
   * We need 2 PDTs: one for us */
  return 5 + kernel_size / PAGE_SIZE + global_options.bss_pages;
}

static uint32_t alloc_page(uint32_t *cur_alloc)
{
  uint32_t r = *cur_alloc;
  *cur_alloc += PAGE_SIZE;
  return r;
}

static void map_page(uint64_t *pdt, uint64_t real_addr, uint64_t virt_addr, uint16_t access_bits, uint32_t *current_page)
{
  uint32_t lo_part = virt_addr & 0xFFFFFFFF;
  uint32_t pdt_entry = (lo_part & 0x3FE00000) >> 21;
  uint32_t pt_entry  = (lo_part & 0x001FF000) >> 12;

  uint64_t *pt = (uint64_t *) (uint32_t) (pdt[pdt_entry] & 0xFFFFFFFFFFFFF000);

  if (!pt) {
    pt = (uint64_t *) alloc_page(current_page);
    pdt[pdt_entry] = ((uint32_t) pt | PAGE_ACCESS_BITS);
  }

  pt[pt_entry] = real_addr | access_bits;
}

static void build_page_tables()
{
  uint32_t current_page = global_options.page_table_start;

  uint32_t pml4_addr = alloc_page(&current_page);
  uint32_t pdpt_addr = alloc_page(&current_page);
  uint32_t pdt_addr = alloc_page(&current_page);

  uint64_t *pml4 = (uint64_t *) pml4_addr;
  uint64_t *pdpt = (uint64_t *) pdpt_addr;
  uint64_t *pdt  = (uint64_t *) pdt_addr;

  int i = 0;

  Elf64_Ehdr *hdr = (Elf64_Ehdr *) global_options.kernel_start;

  global_options.pml4_ptr = pml4_addr;

  /* We ignore the first entry, because it should be 0. */
  /* We parse all sections, but we only care about .bss, .text, .data, and .rodata */
  for (i = 1; i < hdr->e_shnum; ++i) {
    Elf64_Shdr *section = (Elf64_Shdr *) (uint32_t) (global_options.kernel_start + hdr->e_shoff + i * hdr->e_shentsize);

    int is_bss =
      /* .bss */
      ((section->sh_type == SHT_NOBITS) && (section->sh_flags & SHF_ALLOC) &&
       (section->sh_flags & SHF_WRITE));
    int will_load_not_bss =
      /* .data */
      ((section->sh_type == SHT_PROGBITS) && (section->sh_flags & SHF_ALLOC) &&
       (section->sh_flags & SHF_WRITE)) ||
      /* .rodata */
      ((section->sh_type == SHT_PROGBITS) && (section->sh_flags & SHF_ALLOC)) ||
      /* .text */
      ((section->sh_type == SHT_PROGBITS) && (section->sh_flags & SHF_ALLOC) &&
       (section->sh_flags && SHF_EXECINSTR));

    if (will_load_not_bss || is_bss) {
      /* Map this page into memory */
      uint64_t section_real_addr = section->sh_offset + global_options.kernel_start;
      uint64_t section_virt_addr = section->sh_addr;

      uint64_t section_size_in_pages = ALIGN_4K_UP(section->sh_size) / PAGE_SIZE;

      uint16_t access_bits = PAGE_ACCESS_PRESENT;

      if (section->sh_flags & SHF_WRITE)
        access_bits |= PAGE_ACCESS_WRITABLE;

      if (will_load_not_bss)
        for (;section_size_in_pages > 0;
             --section_size_in_pages,
               section_virt_addr += PAGE_SIZE,
               section_real_addr += PAGE_SIZE)
          map_page(pdt, section_real_addr, section_virt_addr, access_bits, &current_page);
      else if (is_bss)
        for (;section_size_in_pages > 0;
             --section_size_in_pages,
               section_virt_addr += PAGE_SIZE) {
          section_real_addr = alloc_page(&current_page); /* BSS pages are uninitialized */
          map_page(pdt, section_real_addr, section_virt_addr, access_bits, &current_page);
        }
    }
  }

  pml4[PML4_ENTRY] = pdpt_addr | PAGE_ACCESS_BITS;
  pdpt[0] = pdt_addr | PAGE_ACCESS_BITS;

  /* Now build the pages for our own code, so that we're still mapped when we jump to long mode */
  pdpt_addr = alloc_page(&current_page);
  pdt_addr = alloc_page(&current_page);

  pdpt = (uint64_t *) pdpt_addr;
  pdt = (uint64_t *) pdt_addr;

  pml4[0] = pdpt_addr | PAGE_ACCESS_BITS;
  pdpt[0] = pdt_addr | PAGE_ACCESS_BITS;
  /* Map in the first four megabytes */
  pdt[0] = 0x80 | PAGE_ACCESS_BITS;
  pdt[1] = 0x200000 | 0x80 | PAGE_ACCESS_BITS;
}

static void reserve_kernel_space(multiboot_info_t *multiboot, int pages_needed)
{
  multiboot_mmap_entry_t *entry;

#ifdef DEBUG
  printf("Memory layout:\n");
#endif

  for (entry = (multiboot_mmap_entry_t *) multiboot->mmap_addr;
       ((unsigned) entry) < (((unsigned) multiboot->mmap_addr) + multiboot->mmap_length);
       entry = (multiboot_mmap_entry_t *) ((unsigned long) entry + entry->size + sizeof(entry->size))) {
    int page_length = ALIGN_4K_DOWN(entry->base_addr + entry->length) - /* Only take the length that can be used if */
      ALIGN_4K_UP(entry->base_addr);                                    /* everything was page aligned */
    int region_page_count = page_length / PAGE_SIZE;
    uint32_t base_addr = entry->base_addr;

    #ifdef DEBUG
    printf("    - %s: 0x%lX -- 0x%lX (%d page(s), 0x%lX bytes)\n",
           entry->type == 1 ? "Available":"Reserved", /* Value of 1 indicates free ram, according to
                                                       * http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
                                                       */
           entry->base_addr,
           (entry->base_addr + entry->length), /* Memory limits */
           region_page_count,
           entry->length);
    #endif

    /* We also have to make sure that we only use space that doesn't contain our own code or our
     * modules. To do this, we make sure that we only ever use space that is greater than the
     * highest module end address */
    if (entry->base_addr < global_options.highest_module_address) {
      base_addr = global_options.highest_module_address;
      page_length = ALIGN_4K_DOWN(entry->base_addr + entry->length) -
        ALIGN_4K_UP(base_addr);
      region_page_count = page_length / PAGE_SIZE;
    }

    if (entry->type == 1 && pages_needed < region_page_count) { /* Make sure we only use free regions with enough pages */
      /* We've found a region that can fit. We always prefer the last region we find, which is why we don't break. */
      global_options.page_table_start = ALIGN_4K_UP(base_addr);
      global_options.page_table_end = global_options.page_table_start + pages_needed * PAGE_SIZE;
    }
  }

  if (!global_options.page_table_start && !global_options.page_table_end)
    error("Couldn't allocate enough space for page tables");

  memset((void *) global_options.page_table_start, 0, pages_needed * PAGE_SIZE);
}

static void verify_elf64()
{
  Elf64_Ehdr *hdr = (Elf64_Ehdr *) global_options.kernel_start;
  static const unsigned char ELF_HDR[] = {0x7f, 0x45, 0x4C, 0x46};
  if (memcmp(hdr->e_ident, ELF_HDR, 4) == 0)
    printf("Kernel is in elf file format...\n");

  /* Do sanity checks on kernel executable */
  if (hdr->e_type != ET_EXEC)
    error("Kernel is not an executable!");
  if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
    error("Kernel is not a 64-bit executable!");
  if (hdr->e_ident[EI_DATA] != ELFDATA2LSB)
    error("Kernel is not little-endian!");
#if DEBUG
  if (hdr->e_ident[EI_VERSION] != EV_CURRENT || hdr->e_version != EV_CURRENT)
    printf("Warning: Kernel uses elf version %u, we're expecting %u\n", hdr->e_ident[EI_VERSION], EV_CURRENT);
#endif

  if (hdr->e_machine != EM_X86_64)
    error("Kernel is not compiled for x86-64!");

#if DEBUG
  printf("Kernel entry point is at 0x%lX\n", hdr->e_entry);
#endif
  kernel_entry = hdr->e_entry;
}

static void load_kernel(multiboot_info_t *multiboot)
{
  int pages = 0;

  if (!global_options.kernel_start || !global_options.kernel_end)
    error("Kernel image not found");

#ifdef DEBUG
  printf("Found kernel at 0x%X -- 0x%X\n", global_options.kernel_start, global_options.kernel_end);
#endif

  verify_elf64();

  pages = estimate_pages_needed_for_kernel();

#ifdef DEBUG
  printf("Going to reserve %d pages for kernel...\n", pages);
#endif

  /* Find space for the kernels page tables and create those tables */
  reserve_kernel_space(multiboot, pages);
  build_page_tables();

#ifdef DEBUG
  printf("The bootstrap page tables will be located at 0x%X -- 0x%X.\n",
         global_options.page_table_start, global_options.page_table_end);
#endif
}

void initMultiboot64(multiboot_info_t *multiboot)
{
  reset_terminal();

  puts("Haskell OS - x86_64 bootstrap code running\n");

  check_multiboot(multiboot);

#ifdef DEBUG
  if (multiboot->flags & MULTIBOOT_BOOT_LDR_NAME_FLAG)
    printf("Booted by %s\n", multiboot->boot_loader_name);
#endif

  if (multiboot->flags & MULTIBOOT_CMDLINE_FLAG)
    parse_cmdline(multiboot);

  /* Finds kernel and sets the information in the boot options structure */
  parse_modules(multiboot);

  load_kernel(multiboot);

  /* Now we actually do the booting */
  asm(
      "jmp _enterLongMode"
      ::
       "D"(global_options.pml4_ptr),
       "b"(multiboot)
      );
}
