/* This file is part of HOS, the Haskell operating system.
 *
 * Copyright (c) 2014 Travis Athougies
 * All rights reserved.
 *
 * This file is under a proprietary licence.
 */
#include <common/common.h>
#include <i386/terminal.h>
#include <i386/stdint.h>
#include <string.h>
#include <stdlib.h>

#define HEIGHT 25
#define WIDTH 80
#define LINE_SIZE WIDTH * 2

int g_row, g_column;

/* White background, black text */
uint8_t g_color_byte = WHITE << 4 | BLACK;

char *g_text_video_memory = (char *) 0xB8000;

void reset_terminal()
{
  /* clear */
  int i = HEIGHT * WIDTH;
  while (i--) {
    g_text_video_memory[i * 2] = ' ';
    g_text_video_memory[i * 2 + 1] = g_color_byte;
  }
}

void set_fg_color(color_t c)
{
  g_color_byte = (g_color_byte & 0xf0) | c;
}

void set_bg_color(color_t c)
{
  g_color_byte = (g_color_byte & 0xf) | (c << 4);
}

static void newline()
{
  if (g_row++ >= HEIGHT) {
    int i;
    char *last_line = g_text_video_memory + (HEIGHT * WIDTH * 2 - LINE_SIZE);
    memcpy(g_text_video_memory, g_text_video_memory + LINE_SIZE, HEIGHT * WIDTH * 2 - LINE_SIZE);
    for (i = 0; i < LINE_SIZE; ++i) {
      last_line[i*2] = ' ';
      last_line[i*2 + 1] = g_color_byte;
    }

    g_row = HEIGHT - 1;
  }
  g_column = 0;
}

void putc(char c)
{
  if (c == '\n')
    newline();
  else {
    g_text_video_memory[g_row * LINE_SIZE + g_column * 2] = c;
    g_text_video_memory[g_row * LINE_SIZE + g_column * 2 + 1] = g_color_byte;

    if (g_column++ >= WIDTH)
      newline();
  }
}

int puts(const char *test)
{
  int ret = 0;
  for (;*test != '\0';++test, ++ret)
      putc(*test);
  return ret;
}

int putint(int i)
{
  char buf[16];
  formatint(i, 10, 0, buf);
  return puts(buf);
}

int putint16(int i)
{
  char buf[16];
  formatint(i, 16, 0, buf);
  return puts(buf);
}

int putuint(unsigned int i)
{
  char buf[16];
  formatunsigned(i, 10, 0, buf);
  return puts(buf);
}

int putuint16(unsigned int i)
{
  char buf[16];
  formatunsigned(i, 16, 0, buf);
  return puts(buf);
}

int printf(const char *s, ...)
{
  va_list ap;
  int ret = 0;

  va_start(ap, s);

  for (;*s != '\0';++s) {
    if (*s == '%') {
      char spec = *++s;
      int d;
      unsigned int u;
      unsigned long long llu;
      char *sp;
      char c;
      char buf[16];
      switch (spec) {
      case 'd':
        d = va_arg(ap, int);
        ret += putint(d);
        break;
      case 's':
        sp = va_arg(ap, char*);
        ret += puts(sp);
        break;
      case 'x':
        d = va_arg(ap, int);
        ret += putint16(d);
        break;
      case 'u':
        u = va_arg(ap, unsigned int);
        ret += putuint(u);
        break;
      case 'X':
        u = va_arg(ap, unsigned int);
        ret += putuint16(u);
        break;
      case '0':
        spec = *++s;
        switch (spec) {
        case 'x':
          d = va_arg(ap, int);
          formatint(d, 16, 1, buf);
          ret += puts(buf);
          break;
        case 'X':
          u = va_arg(ap, unsigned int);
          formatunsigned(u, 16, 1, buf);
          ret += puts(buf);
          break;
        case 'd':
          d = va_arg(ap, int);
          formatint(d, 10, 1, buf);
          ret += puts(buf);
          break;
        case 'u':
          u = va_arg(ap, unsigned int);
          formatunsigned(u, 16, 1, buf);
          ret += puts(buf);
          break;
        default: ++ret; putc('?');
        }
        break;
      case 'l':
        spec = *++s;
        switch (spec) {
        case 'X':
          llu = va_arg(ap, unsigned long long);
          d = formatunsigned(llu >> 32, 16, 0, buf);
          if ((llu >> 32) > 0)
            ret += puts(buf);
          formatunsigned(llu & 0xFFFFFFFF, 16,
                         (llu >> 32) > 0, buf);
          ret += puts(buf);
          break;
        default: ++ret, putc('?');
        }
        break;
      case 'c':
        c = va_arg(ap, int);
        putc(c);
        ++ret;
        break;
      default: ++ret; putc('?');
      }
    } else {
      ++ret;
      putc(*s);
    }
  }

  va_end(ap);
  return ret;
}
