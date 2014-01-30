/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>

static char digits[] = "0123456789"
  "abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "+/"; /* Allows up to base64 encoding */

static int powers[65] = { -1, -1, /* no base 0 or 1 */
                          32,
                          20, 16,
                          13, 12, 11, 10,
                          10, 9, 9, 8, 8, 8, 8, 8,
                          7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                          6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5,
                          5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

int formatint(int i, int base, int pad_zeros, char *buffer)
{
  if (i < 0) {
    int unsigned_length = formatunsigned((unsigned int) -i, base, pad_zeros, buffer);
    if (unsigned_length > 0) {
      memcpy(buffer + 1, buffer, unsigned_length + 1);
      buffer[0] = '-';
    }
    return unsigned_length;
  } else
    return formatunsigned((unsigned int) i, base, pad_zeros, buffer);
}

int formatunsigned(unsigned int i, int base, int pad_zeros, char *buffer)
{
  unsigned int divider = 0;
  int started = pad_zeros;
  int length = 0;
  if (base < 2 || base > 64)
    return -1;

  if (i == 0 && !pad_zeros) {
    buffer[0] = '0';
    buffer[1] = '\0';
    return 1;
  }

  divider = ipow(base, powers[base] - 1);

  do {
    int next_digit = i / divider;

    if (!started && next_digit > 0)
      started = 1;

    if (started) {
      buffer[length] = digits[next_digit];
      length++;
    }

    i %= divider;
    divider /= base;
  } while (divider != 0);

  buffer[length] = '\0';
  return length;
}

/* Implementation taken from http://clc-wiki.net/wiki/C_standard_library:string.h:strcmp */
int strcmp(const char *a1, const char *a2)
{
  while (*a1 && (*a1 == *a2))
    ++a1,++a2;
  return *(const unsigned char*)a1 - *(const unsigned char*)a2;
}

/* Implementation taken from http://clc-wiki.net/wiki/C_standard_library:string.h:strtok */
char *strtok_r(char * str, const char * delim, char **saveptr)
{
  if(str)
    *saveptr=str;
  else if(!*saveptr)
    return 0;
  str=*saveptr+strspn(*saveptr,delim);
  *saveptr=str+strcspn(str,delim);
  if(*saveptr==str)
        return *saveptr=0;
  *saveptr = **saveptr ? **saveptr=0,*saveptr+1 : 0;
  return str;
}

/* Implementation taken from http://clc-wiki.net/wiki/C_standard_library:string.h:strspn */
size_t strspn(const char *s1, const char *s2)
{
  size_t ret=0;
  while(*s1 && strchr(s2,*s1++))
    ret++;
  return ret;
}

/* Implementation taken from http://clc-wiki.net/wiki/C_standard_library:string.h:strcspn */
size_t strcspn(const char *s1, const char *s2)
{
  size_t ret=0;
  while(*s1)
    if(strchr(s2,*s1))
      return ret;
    else
      s1++,ret++;
  return ret;
}

/* Implementation taken from http://clc-wiki.net/wiki/C_standard_library:string.h:strspn */
char *strchr(const char *s, int c)
{
  while (*s != (char)c)
    if (!*s++)
      return 0;
  return (char *)s;
}

/* Implementation taken from http://clc-wiki.net/wiki/C_standard_library:string.h:memcmp */
int memcmp(const void *s1, const void *s2, size_t n)
{
  unsigned char u1, u2;

  for ( ; n-- ; s1++, s2++) {
    u1 = * (unsigned char *) s1;
    u2 = * (unsigned char *) s2;
    if ( u1 != u2) {
      return (u1-u2);
    }
  }
  return 0;
}
