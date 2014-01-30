/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */

#include <common/common.h>
#include <common/options.h>
#include <common/string.h>

static int parse_option(char *option, char **option_name, char **option_value)
{
  char *saveptr;
  *option_name = strtok_r(option, "=", &saveptr);
  *option_value = strtok_r(NULL, "=", &saveptr);
  return 1;
}

static const option_t *find_option(const option_t *options, char *option)
{
  for (;options->name && options->cb;++options)
    if (strcmp(option, options->name) == 0)
      return options;
  set_error("Encountered unknown option");
  return NULL;
}

int parse_command_line(char *command_line, const option_t *options, void *data)
{
  char *saveptr;
  char *option;
  char *option_name, *option_value;

  for (option = strtok_r(command_line, " ", &saveptr); option; option = strtok_r(NULL, " ", &saveptr)) {
    const option_t *option_structure;
    int option_succeeded = parse_option(option, &option_name, &option_value);
    if (!option_succeeded)
      return 0;

    option_structure = find_option(options, option);
    if (!option_structure)
      return 0;

    if (!option_structure->cb) {
      set_error("Invalid callback given in option structure");
      return 0;
    }
    if (!option_structure->cb(option_name, option_value, data)) {
      return 0;
    }
  }
  return 1;
}
