#include <stdarg.h>
#include "printk.h"
#include "lib.h"
#include "font.h"

// global variable
// extern unsigned char font_ascii[256][16];
char buf[4096] = {0};
extern struct position pos;

/**
 * convert numeric letters to integer values
 * @param s string address of numeric
 * @return number after conversion
 */
int skip_atoi(const char **s) {
  int i = 0;
  while (is_digit(**s)) {
    i = i * 10 + *((*s)++) - '0';
  }
  return i;
}

/**
 * print character of paramters required
 * @param fb
 * @param x_size
 * @param x
 * @param y
 * @param fb_color
 * @param bk_color
 * @param font
 * @return
 */
void putchar(unsigned int *fb, int x_size, int x, int y, unsigned int fb_color,
             unsigned bk_color, unsigned char font) {
  int i = 0, j = 0;
  unsigned int *addr = NULL;
  unsigned char *fontp = NULL;
  fontp = font_ascii[font];
  int testval;
  for (i = 0; i < 16; i++) {
    addr = fb + x_size * (y + i) + x;
    testval = 0x100;
    for (j = 0; j < 8; j++) {
      testval = testval >> 1;
      if (*fontp & testval) {
        *addr = fb_color;
      } else {
        *addr = bk_color;
      }
      addr++;
    }
    fontp++;
  }
}

/**
 * converts the value of a long integer variable to a string with a spacified
 * hexadecimal spacifition
 *
 * @param str buffer of save parse result
 * @param num convert variable
 * @param base base number
 * @param field_width
 * @param precision dipaly precision
 * @param flags
 * @return
 */
static char *number(char *str, long num, int base, int field_width,
                    int precision, int flags) {
  char c, sign, tmp[50];
  const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int i;
  if (base < 2 || base > 36) {
    return NULL;
  }
  if (flags & SMALL) {
    digits = "0123456789abcdefghijklmnopqrstuvwxyz";
  }
  if (flags & LEFT) {
    flags &= ~ZEROPAD;
  }
  c = (flags & ZEROPAD) ? '0' : ' ';
  sign = 0;
  if (flags & SIGN && num < 0) {
    sign = '-';
    num = -num;
  } else {
    sign = (flags & PLUS) ? '+' : ((flags & SPACE) ? ' ' : 0);
  }
  if (sign) {
    field_width--;
  }
  if (flags & SPECIAL) {
    if (base == 16) {
      field_width -= 2;
    } else if (base == 8) {
      field_width--;
    }
  }
  i = 0;
  if (num == 0) {
    tmp[i++] = '0';
  } else {
    while (num != 0) {
      tmp[i++] = digits[do_div(num, base)];
    }
  }
  if (i > precision) {
    precision = i;
  }
  field_width -= precision;
  if (!(flags & (ZEROPAD + LEFT))) {
    while (field_width-- > 0) {
      *str++ = ' ';
    }
  }
  if (sign) {
    *str++ = sign;
  }
  if (flags & SPECIAL) {
    if (base == 8) {
      *str++ = '0';
    } else if (base == 16) {
      *str++ = '0';
      *str++ = digits[33];
    }
  }
  if (!(flags & LEFT)) {
    while (field_width-- > 0) {
      *str++ = c;
    }
  }
  while (i < precision--) {
    *str++ = '0';
  }
  while (i-- > 0) {
    *str++ = tmp[i];
  }
  while (field_width-- > 0) {
    *str++ = ' ';
  }
  return str;
}

/**
 * parse the formatted string and paramters provided by color_printk function
 * @param buf buffer of save parse result
 * @param fmt the format
 * @param args the paramters
 * @return string length
 */
int vsprintf(char *buf, const char *fmt, va_list args) {
  char *str, *s;
  int flags;
  int field_width;
  int precision;
  int len, i;

  int qualifier;

  for (str = buf; *fmt; fmt++) {
    if (*fmt != '%') {
      *str++ = *fmt;
      continue;
    }
    flags = 0;
  repeat:
    fmt++;
    switch (*fmt) {
    case '-':
      flags |= LEFT;
      goto repeat;
    case '+':
      flags |= PLUS;
      goto repeat;
    case ' ':
      flags |= SPACE;
      goto repeat;
    case '#':
      flags |= SPECIAL;
      goto repeat;
    case '0':
      flags |= ZEROPAD;
      goto repeat;
    }
    // get fied with
    field_width = -1;
    if (is_digit(*fmt)) {
      field_width = skip_atoi(&fmt);
    } else if (*fmt == '*') {
      fmt++;
      field_width = va_arg(args, int);
      if (field_width < 0) {
        field_width = -field_width;
        flags |= LEFT;
      }
    }
    precision = -1;
    if (*fmt == '.') {
      fmt++;
      if (is_digit(*fmt)) {
        precision = skip_atoi(&fmt);
      } else if (*fmt == '*') {
        fmt++;
        field_width = va_arg(args, int);
      }
      if (field_width < 0) {
        precision = 0;
      }
    }
    qualifier = -1;
    if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z') {
      qualifier = *fmt;
      fmt++;
    }
    switch (*fmt) {
    // char
    case 'c':
      if (!(flags & LEFT)) {
        // on the right
        while (--field_width > 0) {
          *str++ = ' ';
        }
      }
      *str++ = (unsigned char)va_arg(args, int);
      while (--field_width > 0) {
        *str++ = ' ';
      }
      break;
    // string
    case 's':
      s = va_arg(args, char *);
      if (!s) {
        *s = '\0';
      }
      len = strlen(s);
      if (precision < 0) {
        precision = len;
      } else if (len > precision) {
        len = precision;
      }

      if (!(flags & LEFT)) {
        while (len < field_width--) {
          *str++ = ' ';
        }
      }
      for (i = 0; i < len; i++) {
        // copy the string to the buffer
        *str++ = *s++;
      }
      while (len < field_width--) {
        *str++ = ' ';
      }
      break;
    // octal number
    case 'o':
      if (qualifier == 'l') {
        str = number(str, va_arg(args, unsigned long), 8, field_width,
                     precision, flags);
      } else {
        str = number(str, va_arg(args, unsigned int), 8, field_width, precision,
                     flags);
      }
      break;
    // address
    case 'p':
      if (field_width == -1) {
        field_width = 2 * sizeof(void *);
        flags |= ZEROPAD;
      }
      str = number(str, (unsigned long)va_arg(args, void *), 16, field_width,
                   precision, flags);
      break;
    case 'x':
      flags |= SMALL;
      break;
    case 'X':
      if (qualifier == 'l') {
        str = number(str, va_arg(args, unsigned long), 16, field_width,
                     precision, flags);
      } else {
        str = number(str, va_arg(args, unsigned int), 16, field_width,
                     precision, flags);
      }
      break;
    case 'd':
    case 'i':
      flags |= SIGN;
      break;
    case 'u':
      if (qualifier == 'l') {
        str = number(str, va_arg(args, unsigned long), 10, field_width,
                     precision, flags);
      } else {
        str = number(str, va_arg(args, unsigned int), 10, field_width,
                     precision, flags);
      }
      break;
    case 'n':
      if (qualifier == 'l') {
        long *ip = va_arg(args, long *);
        *ip = (str - buf);
      } else {
        int *ip = va_arg(args, int *);
        *ip = (str - buf);
      }
      break;
    // if appear %%, the first % as an escape character, after formatting, only
    // one character % */
    case '%':
      *str++ = '%';
      break;
    // if not support, direct output without any processing
    default:
      *str++ = '%';
      if (*fmt) {
        *str++ = *fmt;
      } else {
        fmt--;
      }
      break;
    }
  }
  *str = '\0';
  return str - buf;
}

int color_printk(unsigned int fr_color, unsigned int bk_color, const char *fmt,
                 ...) {
  int i = 0;
  int count = 0;
  int line = 0;
  va_list args;
  va_start(args, fmt);
  i = vsprintf(buf, fmt, args);
  va_end(args);

  for (count = 0; count < i || line; count++) {
    if (line > 0) {
      count--;
      goto label_tab;
    }
    if ((unsigned char)*(buf + count) == '\n') {
      pos.y_position++;
      pos.x_position = 0;
    } else if ((unsigned char)*(buf + count) == '\b') {
      pos.x_position--;
      if (pos.x_position < 0) {
        pos.x_position = pos.x_resolution / pos.x_char_size - 1;
        pos.y_position--;
        if (pos.y_position < 0) {
          pos.y_position = pos.y_position * pos.x_char_size - 1;
        }
      }
      putchar(pos.fb_addr, pos.x_resolution, pos.x_position * pos.x_char_size,
              pos.y_position * pos.y_char_size, fr_color, bk_color, ' ');
    } else if ((unsigned char)*(buf + count) == '\t') {
      line = ((pos.x_position + 8) & ~(8 - 1)) - pos.x_position;
    label_tab:
      line--;
      putchar(pos.fb_addr, pos.x_resolution, pos.x_position * pos.x_char_size,
              pos.y_position * pos.y_char_size, fr_color, bk_color, ' ');
      pos.x_position++;
    } else {
      putchar(pos.fb_addr, pos.x_resolution, pos.x_position * pos.x_char_size,
              pos.y_position * pos.y_char_size, fr_color, bk_color,
              (unsigned char)*(buf + count));
      pos.x_position++;
    }
    if (pos.x_position >= (pos.x_resolution / pos.x_char_size)) {
      pos.y_position++;
      pos.x_position = 0;
    }
    if (pos.y_position >= (pos.y_resolution / pos.y_char_size)) {
      pos.y_position = 0;
    }
  }
  return i;
}
