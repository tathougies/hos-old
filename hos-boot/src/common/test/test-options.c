/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#include <common/options.h>
#include <common/common.h>

#undef NULL
#include <stdio.h>

static char *kernel_name = "Interlude";

static int set_kernel(char *name, char *value, void *unused)
{
  if (!value) {
    set_error("kernel option expects value");
    return 0;
  }

  kernel_name = value;
  return 1;
}

static option_t options[] = {
  {"kernel", set_kernel},
  {NULL, NULL}
};

int main(int argc, char **argv)
{
  char *line;
  int result;
  if (argc != 2) {
    printf("test-options OPTION-STRING\n");
    return 1;
  }

  line = argv[1];

  result = parse_command_line(line, options, NULL);
  if (!result) {
    printf("Encountered error while parsing: %s\n", get_error());
    return 1;
  }

  printf("Kernel name: %s\n", kernel_name);

  return 0;
}
