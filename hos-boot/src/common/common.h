/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#ifndef __common_common_h__
#define __common_common_h__

#define NULL ((void *) 0)

#define PACKED __attribute__ ((__packed__))

#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_copy(d, s) __builtin_va_copy(d, s)

typedef __builtin_va_list va_list;

/* Alignment macros */
#define ALIGN_4K_UP(x) (((x) + 0x1000 - 1) & ~0xFFF)
#define ALIGN_4K_DOWN(x) ((x) & ~0xFFF)

#define ALIGNED_2MB(x) (!((x) & 0x1FFFFF))
#define ALIGNED_4K(x) (!((x) & 0xFFF))

void set_error(const char *error_msg);
const char *get_error();

#endif
