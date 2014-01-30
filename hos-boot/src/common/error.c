/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */

#include <common/common.h>

static const char *g_error;

void set_error(const char *error)
{
  g_error = error;
}

const char *get_error()
{
  return g_error;
}
