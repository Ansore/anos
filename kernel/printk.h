#ifndef __PRINTK_H__
#define __PRINTK_H__

#include <stdarg.h>

#define ZEROPAD 1  // pad with zero
#define SIGN 2     // unsigned/signed long
#define PLUS 4     // show plus
#define SPACE 8    // space if plus
#define LEFT 16    // left justified
#define SPACIAL 32 // 0x
#define SMALL 64   // use 'abcdef' instead of 'ABCDEF'

#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define do_div(n, base)                                                        \
  ({                                                                           \
    int __res;                                                                 \
    __asm__("divq %%rcx" : "=a"(n), "=d"(__res) : "0"(n), "1"(0), "c"(base));  \
  })

#define WHITE 0x00ffffff  // white
#define BLACK 0x00000000  // black
#define RED 0x00ff0000    // red
#define ORANGE 0x00ff8000 // orange
#define YELLOW 0x00ffff00 // yellow
#define GREEN 0x0000ff00  // green
#define BLUE 0x000000ff   // blue
#define INDIGO 0x0000ffff // indigo
#define PURPLE 0x008000ff // purple

struct position {
  int x_resolution;
  int y_resolution;

  int x_position;
  int y_position;

  int x_char_size;
  int y_char_size;

  unsigned int *fb_addr;
  unsigned long fb_length;
};

void putchar(unsigned int *fb, int x_size, int x, int y, unsigned int fb_color,
             unsigned bk_color, unsigned char font);

int skip_atoi(const char **s);

static char *number(char *str, long num, int base, int size, int precision,
                    int type);

int vsprintf(char *buf, const char *fmt, va_list args);

int color_printk(unsigned int fr_color, unsigned int bk_color, const char *fmt,
                 ...);

#endif
