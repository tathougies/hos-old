/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#ifndef __common_stdlib_h__
#define __common_stdlib_h__

#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int val, size_t len);

void abort();

#endif
