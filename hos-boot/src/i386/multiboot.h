/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#ifndef __i386_multiboot_h__
#define __i386_multiboot_h__

#include "common/common.h" /* For PACKED */
#include "i386/stdint.h"

#define MULTIBOOT_MEM_LIMITS_FLAG      0x00000001
#define MULTIBOOT_BOOT_DEV_FLAG        0x00000002
#define MULTIBOOT_CMDLINE_FLAG         0x00000004
#define MULTIBOOT_MODULES_FLAG         0x00000008
#define MULTIBOOT_SYMBOLS_AOUT_FLAG    0x00000010
#define MULTIBOOT_SYMBOLS_ELF_FLAG     0x00000020
#define MULTIBOOT_MEM_MAP_FLAG         0x00000040
#define MULTIBOOT_DRIVES_FLAG          0x00000080
#define MULTIBOOT_CONFIG_TABLE_FLAG    0x00000100
#define MULTIBOOT_BOOT_LDR_NAME_FLAG   0x00000200
#define MULTIBOOT_APM_TABLE_FLAG       0x00000400
#define MULTIBOOT_VBE_INFO_FLAG        0x00000800

typedef struct PACKED {
  uint32_t tabsize, strsize;
  uint32_t addr;
  uint32_t reserved;
} aout_syms_t;

typedef struct PACKED {
  uint32_t num, size;
  void *addr;
  uint32_t shndx;
} elf_syms_t;

typedef struct PACKED {
  uint32_t mod_start, mod_end;
  char *mod_name;
  uint32_t reserved;
} multiboot_module_t;

typedef struct PACKED {
  uint32_t size;
  uint64_t base_addr;
  uint64_t length;
  uint32_t type;
} multiboot_mmap_entry_t;

typedef struct PACKED {
  uint32_t flags; /* This is the only field that is guaranteed to be filled in */

  /* The following fields may or may not be set, depending on the appropriate value in flags */

  /* MULTIBOOT_MEM_LIMITS_FLAG must be set */
  uint32_t mem_limit_low, mem_limit_high;

  /* MULTIBOOT_BOOT_DEV_FLAG must be set */
  uint8_t boot_device_part3, boot_device_part2, boot_device_part1, boot_device_drive;

  /* MULTIBOOT_CMDLINE_FLAG must be set */
  char *command_line;

  /* MULTIBOOT_MODULES_FLAG must be set */
  uint32_t mods_count;
  multiboot_module_t *mods;

  /* MULTIBOOT_SYMBOLS_AOUT_FLAG or MULTIBOOT_SYMBOLS_ELF_FLAG must be set */
  union {
    aout_syms_t aout_syms; /* MULTIBOOT_SYMBOLS_AOUT_FLAG */
    elf_syms_t elf_syms;   /* MULTIBOOT_SYMBOLS_ELF_FLAG */
  } syms;

  /* MULTIBOOT_MEM_MAP_FLAG must be set */
  uint32_t mmap_length;
  void *mmap_addr;

  /* MULTIBOOT_DRIVES_FLAG must be set */
  uint32_t drives_length;
  void *drives_addr;

  /* MULTIBOOT_CONFIG_TABLE_FLAG must be set */
  void *config_table;

  /* MULTIBOOT_BOOT_LDR_NAME_FLAG must be set */
  char *boot_loader_name;

  /* MULTIBOOT_APM_TABLE_FLAG must be set */
  void *apm_table;

  /* MULTIBOOT_VBE_INFO_FLAG must be set */
  void *vbe_control_info, *vbe_mode_info;
  uint32_t vbe_mode;
  uint32_t vbe_interface_seg, vbe_interface_off, vbe_interface_len;

} multiboot_info_t;

#endif
