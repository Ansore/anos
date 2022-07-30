#include "memory.h"
#include "lib.h"
#include "printk.h"

void memory_init() {
  int i, j;
  unsigned long total_memory = 0;
  struct E820 *p = NULL;

  color_printk(BLUE, BLACK,
               "Display Physics Adress MAP, Type(1:RAM,2:ROM or Reserved, "
               "3:ACPI Reclaim Memory, 4:ACPI NVS Memory, Others:Undefine)\n");
  p = (struct E820 *)0xffff800000007e00;
  for (i = 0; i < 32; i++) {
    color_printk(ORANGE, BLACK,
                 "Address:%#018lx\tLength:%#018lx\tType:%#010x\n", p->address,
                 p->length, p->type);
    if (p->type == 1) {
      total_memory += p->length;
    }

    memory_management_struct.e820[i].address += p->address;
    memory_management_struct.e820[i].length += p->length;
    memory_management_struct.e820[i].type += p->type;
    memory_management_struct.e820_length = i;

    p++;
    if (p->type > 4) {
      break;
    }
  }
  color_printk(ORANGE, BLACK, "OS can used total ram: %018lx\n", total_memory);

  total_memory = 0;
  for (i = 0; i <= memory_management_struct.e820_length; i++) {
    unsigned long start, end;
    if (memory_management_struct.e820[i].type != 1) {
      continue;
    }
    start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
    end = ((memory_management_struct.e820[i].address +
            memory_management_struct.e820[i].length) >>
           PAGE_2M_SHIFT)
          << PAGE_2M_SHIFT;
    if (end <= start) {
      continue;
    }
    total_memory += (end - start) >> PAGE_2M_SHIFT;
  }

  color_printk(ORANGE, BLACK, "OS can used total 2M PAGEs: %#010x=%010d\n",
               total_memory, total_memory);
}
