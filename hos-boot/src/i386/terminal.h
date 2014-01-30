/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#ifndef __i386_terminal_h__
#define __i386_terminal_h__

typedef enum {
  BLACK = 0,
  BLUE = 1,
  GREEN = 2,
  CYAN = 3,
  RED = 4,
  MAGENTA = 5,
  BROWN = 6,
  LIGHT_GRAY = 7,
  DARK_GRAY = 8,
  LIGHT_BLUE = 9,
  LIGHT_GREEN = 10,
  LIGHT_CYAN = 11,
  LIGHT_RED = 12,
  LIGHT_MAGENTA = 13,
  YELLOW = 14,
  WHITE = 15
} color_t;

void reset_terminal();

void set_fg_color(color_t c);
void set_bg_color(color_t c);

int puts(const char *s);
int putint(int i);
int putint16(int i);
int putuint(unsigned int i);
int putuint16(unsigned int i);
void putc(char c);

int printf(const char *s, ...);

#endif
