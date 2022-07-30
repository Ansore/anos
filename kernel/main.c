#include "gate.h"
#include "memory.h"
#include "printk.h"
#include "trap.h"

struct position pos;
struct global_memory_descriptor memory_management_struct = {{0}, 0};

void start_kernel(void) {
  int *addr = (int *)0xffff800000a00000;
  int i;

  pos.x_resolution = 1440;
  pos.y_resolution = 900;
  pos.x_position = 0;
  pos.y_position = 0;
  pos.x_char_size = 8;
  pos.y_char_size = 16;
  pos.fb_addr = (unsigned int *)0xffff800000a00000;
  pos.fb_length = (pos.x_resolution * pos.y_resolution * 4);

  load_TR(8);
  set_tss64(0xffff800000007c00, 0xffff800000007c00, 0xffff800000a00000,
            0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,
            0xffff800000007c0, 0xffff800000007c00, 0xffff800000007c00,
            0xffff800000007c00);
  sys_vector_init();

  memory_init();

  // i = 1 / 0; // divide 0 exception
  // i = *(int*) 0xffff80000aa00000; // #PF exception
  color_printk(YELLOW, BLACK, "Hello\t\t World!\n");
  color_printk(GREEN, BLACK, "addr test: %p\n", addr);
  color_printk(GREEN, BLACK, "o test: %o\n", 10);
  color_printk(GREEN, BLACK, "hex test: %x\n", 10);
  color_printk(GREEN, BLACK, "integer test: %d\n", 10);
  color_printk(GREEN, BLACK, "char test: %c\n", 'h');
  color_printk(GREEN, BLACK, "string test: %s\n", "str");
  while (1) {
    ;
  }
}
