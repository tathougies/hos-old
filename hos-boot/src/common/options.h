/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#ifndef __common_options_h__
#define __common_options_h__

/* Takes the option name and value and the user data.
 * Returns 1 on success and 0 on error */
typedef int(*option_cb)(char *, char *, void *);

typedef struct {
  char *name; /* The option name (the part before the '=' sign) */
  option_cb cb; /* The function called when the option is encountered */
} option_t;

/* Parses a command line. The command line passed in the first argument will be modified.
 *
 * The options are specified as an array in the options pointer. The last option is given as {name =
 * 0, cb = 0}.
 *
 * Returns 1 on success and 0 on error.
 */
int parse_command_line(char *command_line, const option_t *options, void *data);

#endif
