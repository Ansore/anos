#include "gate.h"
#include "memory.h"
#include "printk.h"
#include "trap.h"

struct position pos;
struct global_memory_descriptor memory_management_struct = {{0}, 0};

extern char _text;
extern char _etext;
extern char _edata;
extern char _end;

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
  pos.fb_length = (pos.x_resolution * pos.y_resolution * 4 + PAGE_4K_SIZE - 1) &
                  PAGE_4K_MASK;

  load_TR(8);
  set_tss64(0xffff800000007c00, 0xffff800000007c00, 0xffff800000a00000,
            0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,
            0xffff800000007c0, 0xffff800000007c00, 0xffff800000007c00,
            0xffff800000007c00);
  sys_vector_init();

  memory_management_struct.start_code = (unsigned long) &_text;
  memory_management_struct.end_code = (unsigned long) &_etext;
  memory_management_struct.end_data = (unsigned long) &_edata;
  memory_management_struct.end_brk = (unsigned long) &_end;

  memory_init();

  while (1) {
    ;
  }
}
