/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#include <stdlib.h>

void *memcpy(void *dest, const void *src, size_t len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}

void *memset(void *dest, int val, size_t len)
{
  unsigned char *ptr = dest;
  while (len--)
    *ptr++ = val;
  return dest;
}
