/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#include <math.h>

int ipow(int base, int e)
{
  int a = 1;
  while (e) {
    if (e & 1)
      a *= base;
    e >>= 1;
    base *= base;
  }
  return a;
}
