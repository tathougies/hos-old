/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#ifndef __common_elf64_h__
#define __common_elf64_h__

#include <common/common.h>
#include <stdint.h>

/* e_ident possibilities */

#define EI_MAG0          0
#define EI_MAG1          1
#define EI_MAG2          2
#define EI_MAG3          3
#define EI_CLASS         4
#define EI_DATA          5
#define EI_VERSION       6
#define EI_OSABI         7
#define EI_ABIVERSION    8
#define EI_PAD           9
#define EI_NIDENT        16

/* Object file classes, e_ident[EI_CLASS] */
#define ELFCLASS32       1
#define ELFCLASS64       2

/* Data encodings, e_ident[EI_DATA] */
#define ELFDATA2LSB      1 /* Little-endian */
#define ELFDATA2MSB      2 /* Big-endian */

/* Current ELF version */
#define EV_CURRENT       1

/* Object file type, e_type */
#define ET_NONE          0
#define ET_REL           1
#define ET_EXEC          2
#define ET_DYN           3
#define ET_CORE          4
#define ET_LOOS          0xFEDO
#define ET_HIOS          0xFEFF
#define ET_LOPROC        0xFF00
#define ET_HIPROC        0xFFFF

/* Machine types */
#define EM_X86_64        62

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

typedef struct
{
  unsigned char e_ident[16];  /* ELF identification */
  Elf64_Half    e_type;       /* Object file type */
  Elf64_Half    e_machine;    /* Machine type */
  Elf64_Word    e_version;    /* Object file version */
  Elf64_Addr    e_entry;      /* Entry point address */
  Elf64_Off     e_phoff;      /* Program header offset */
  Elf64_Off     e_shoff;      /* Section header offset */
  Elf64_Word    e_flags;      /* Processor-specific flags */
  Elf64_Half    e_ehsize;     /* ELF header size */
  Elf64_Half    e_phentsize;  /* Size of program header entry*/
  Elf64_Half    e_phnum;      /* Number of program header entries */
  Elf64_Half    e_shentsize;  /* Size of section header entry */
  Elf64_Half    e_shnum;      /* Number of section header entries */
  Elf64_Half    e_shstrndx;   /* Section name string table index */
} Elf64_Ehdr;

/* Section type */
#define SHT_NULL         0
#define SHT_PROGBITS     1
#define SHT_SYMTAB       2
#define SHT_STRTAB       3
#define SHT_RELA         4
#define SHT_HASH         5
#define SHT_DYNAMIC      6
#define SHT_NOTE         7
#define SHT_NOBITS       8
#define SHT_REL          9
#define SHT_SHLIB        10
#define SHT_DYNSYM       11
#define SHT_LOOS         0x60000000
#define SHT_HIOS         0x6FFFFFFF
#define SHT_LOPROC       0x70000000
#define SHT_HIPROC       0x7FFFFFFF

/* Section flags */
#define SHF_WRITE        0x0001
#define SHF_ALLOC        0x0002
#define SHF_EXECINSTR    0x0004
#define SHF_MASKOS       0x0F000000
#define SHF_MASKPROC     0xF0000000

typedef struct
{
  Elf64_Word    sh_name;      /* Section name */
  Elf64_Word    sh_type;      /* Section type */
  Elf64_Xword   sh_flags;     /* Section attributes */
  Elf64_Addr    sh_addr;      /* Virtual address in memory */
  Elf64_Off     sh_offset;    /* Offset in file */
  Elf64_Xword   sh_size;      /* Size of section */
  Elf64_Word    sh_link;      /* Link to other section */
  Elf64_Word    sh_info;      /* Miscellaneous information */
  Elf64_Xword   sh_addralign; /* Address alignment boundary */
  Elf64_Xword   sh_entsize;   /* Size of entries, if section has table */
} Elf64_Shdr;

#endif
