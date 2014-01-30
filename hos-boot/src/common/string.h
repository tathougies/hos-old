/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#ifndef __common_string_h__
#define __common_string_h__

#include <common/stdlib.h>

int formatint(int i, int base, int pad_zeros, char* buffer);
int formatunsigned(unsigned int i, int base, int pad_zeros, char *buffer);

int strcmp(const char *a1, const char *a2);
char *strtok_r(char *str, const char *delim, char **saveptr);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char *strchr(const char *s, int c);

int memcmp(const void *s1, const void *s2, size_t n);

#define memmove __builtin_memmove
#define strcpy __builtin_strcpy
#define strlen __builtin_strlen
#define strncpy __builtin_strncpy
#define strrchr __builtin_strrchr

#endif
