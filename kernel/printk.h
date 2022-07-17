#ifndef __PRINTK_H__
#define __PRINTK_H__

#define ZEROPAD 1  // pad with zero
#define SIGN 2     // unsigned/signed long
#define PLUS 4     // show plus
#define SPACE 8    // space if plus
#define LEFT 16    // left justified
#define SPACIAL 32 // 0x
#define SMALL 64   // use 'abcdef' instead of 'ABCDEF'

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

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

#endif
