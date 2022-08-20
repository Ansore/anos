#include "gate.h"
#include "interrupt.h"
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
  struct page *page = NULL;

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

  memory_management_struct.start_code = (unsigned long)&_text;
  memory_management_struct.end_code = (unsigned long)&_etext;
  memory_management_struct.end_data = (unsigned long)&_edata;
  memory_management_struct.end_brk = (unsigned long)&_end;

  color_printk(RED, BLACK, "memory init");
  memory_init();

  color_printk(RED, BLACK, "interrupt init");
  interrupt_init();

  /*
  color_printk(RED, BLACK, "memory_management_struct.bits_map:%#018lx\n",
               *memory_management_struct.bits_map);
  color_printk(RED, BLACK, "memory_management_struct.bits_map:%#018lx\n",
               *(memory_management_struct.bits_map + 1));

  page = alloc_pages(ZONE_NOMAL, 64, PG_PTABLE_MAPED | PG_ACTIVE | PG_KERNEL);

  for (i = 0; i <= 64; i++) {
    color_printk(INDIGO, BLACK, "page%d\tattribute:%#018lx\taddress:%#018lx\t",
                 i, (page + i)->attribute, (page + i)->phy_address);
    i++;
    color_printk(INDIGO, BLACK, "page%d\tattribute:%#018lx\taddress:%#018lx\t\n",
                 i, (page + i)->attribute, (page + i)->phy_address);
  }

  color_printk(RED, BLACK, "memory_management_struct.bits_map:%#018lx\n",
               *memory_management_struct.bits_map);
  color_printk(RED, BLACK, "memory_management_struct.bits_map:%#018lx\n",
               *(memory_management_struct.bits_map + 1));
  */

  while (1) {
    ;
  }
}
