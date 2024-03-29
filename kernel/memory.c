#include "memory.h"
#include "lib.h"
#include "printk.h"

unsigned long page_init(struct page *page, unsigned long flags) {
  if (!page->attribute) {
    *(memory_management_struct.bits_map +
      ((page->phy_address >> PAGE_2M_SHIFT) >> 6)) |=
        1UL << (page->phy_address >> PAGE_2M_SHIFT) % 64;
    page->attribute = flags;
    page->reference_count++;
    page->zone_struct->page_using_count++;
    page->zone_struct->page_free_count++;
    page->zone_struct->total_pages_link++;
  } else if ((page->attribute & PG_REFRENCED) ||
             (page->attribute & PG_K_SHARE_TO_U) || (flags & PG_REFRENCED) ||
             (flags & PG_K_SHARE_TO_U)) {
    page->attribute |= flags;
    page->reference_count++;
    page->zone_struct->total_pages_link++;
  } else {
    *(memory_management_struct.bits_map +
      ((page->phy_address >> PAGE_2M_SHIFT) >> 6)) |=
        1UL << (page->phy_address >> PAGE_2M_SHIFT) % 64;
    page->attribute |= flags;
  }
  return 0;
}

unsigned long page_clean(struct page *page) {
  if (!page->attribute) {
    page->attribute = 0;
  } else if ((page->attribute & PG_REFRENCED) ||
             (page->attribute & PG_K_SHARE_TO_U)) {
    page->reference_count--;
    page->zone_struct->total_pages_link--;
    if (!page->reference_count) {
      page->attribute = 0;
      page->zone_struct->page_using_count--;
      page->zone_struct->page_free_count++;
    }
  } else {
    *(memory_management_struct.bits_map +
      ((page->phy_address >> PAGE_2M_SHIFT) >> 6)) &=
        (~1UL << (page->phy_address >> PAGE_2M_SHIFT) % 64);
    page->attribute = 0;
    page->reference_count = 0;
    page->zone_struct->page_using_count--;
    page->zone_struct->page_free_count++;
    page->zone_struct->total_pages_link--;
  }
  return 0;
}

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
    if (p->type > 4 || p->length == 0 || p->type < 1) {
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
  total_memory =
      memory_management_struct.e820[memory_management_struct.e820_length]
          .address +
      memory_management_struct.e820[memory_management_struct.e820_length]
          .length;

  // bits map construction init
  memory_management_struct.bits_map =
      (unsigned long *)((memory_management_struct.end_brk + PAGE_4K_SIZE - 1) &
                        PAGE_4K_MASK);

  memory_management_struct.bits_size = total_memory >> PAGE_2M_SHIFT;

  memory_management_struct.bits_length =
      (((unsigned long)(total_memory >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) /
       8) &
      (~(sizeof(long) - 1));
  memset(memory_management_struct.bits_map, 0xff,
         memory_management_struct.bits_length);

  // page construction init
  memory_management_struct.pages_struct =
      (struct page *)(((unsigned long)memory_management_struct.bits_map +
                       memory_management_struct.bits_length + PAGE_4K_SIZE -
                       1) &
                      PAGE_4K_MASK);
  memory_management_struct.pages_size = total_memory >> PAGE_2M_SHIFT;
  memory_management_struct.pages_length =
      ((total_memory >> PAGE_2M_SHIFT) * sizeof(struct page) + sizeof(long) -
       1) &
      (~(sizeof(long) - 1));
  memset(memory_management_struct.pages_struct, 0x00,
         memory_management_struct.pages_length);

  // zones construction init
  memory_management_struct.zones_struct =
      (struct zone *)(((unsigned long)memory_management_struct.pages_struct +
                       memory_management_struct.pages_length + PAGE_4K_SIZE -
                       1) &
                      PAGE_4K_MASK);

  memory_management_struct.zones_size = 0;
  memory_management_struct.zones_length =
      (5 * sizeof(struct zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));
  memset(memory_management_struct.zones_struct, 0x00,
         memory_management_struct.zones_length);

  for (i = 0; i <= memory_management_struct.e820_length; i++) {
    unsigned long start, end;
    struct zone *z;
    struct page *p;
    unsigned long *b;

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
    // zone init
    z = memory_management_struct.zones_struct +
        memory_management_struct.zones_size;
    memory_management_struct.zones_size++;
    z->zone_start_address = start;
    z->zone_end_address = end;
    z->zone_length = end - start;

    z->page_using_count = 0;
    z->page_free_count = end - start;

    z->total_pages_link = 0;
    z->attribute = 0;
    z->gmd_struct = &memory_management_struct;

    z->pages_length = (end - start) >> PAGE_2M_SHIFT;
    z->pages_group = (struct page *)(memory_management_struct.pages_struct +
                                     (start >> PAGE_2M_SHIFT));

    // page init
    p = z->pages_group;
    for (j = 0; j < z->pages_length; j++, p++) {
      p->zone_struct = z;
      p->phy_address = start + PAGE_2M_SIZE * j;
      p->attribute = 0;
      p->reference_count = 0;
      p->age = 0;
      *(memory_management_struct.bits_map +
        ((p->phy_address >> PAGE_2M_SHIFT) >> 6)) ^=
          1UL << (p->phy_address >> PAGE_2M_SHIFT) % 64;
    }
  }

  // init address 0 to page struct 0; because the
  // memory_management_struct.e820[0].type ! = 1
  memory_management_struct.pages_struct->zone_struct =
      memory_management_struct.zones_struct;
  memory_management_struct.pages_struct->phy_address = 0UL;
  memory_management_struct.pages_struct->attribute = 0;
  memory_management_struct.pages_struct->reference_count = 0;
  memory_management_struct.pages_struct->age = 0;

  memory_management_struct.zones_length =
      (memory_management_struct.zones_size * sizeof(struct zone) +
       sizeof(long) - 1) &
      (~(sizeof(long) - 1));

  color_printk(
      ORANGE, BLACK, "bits_map:%#018lx,bits_size:%#018lx,bits_length:%#018lx\n",
      memory_management_struct.bits_map, memory_management_struct.bits_size,
      memory_management_struct.bits_length);

  color_printk(ORANGE, BLACK,
               "pages_struct:%#018lx,pages_size:%#018lx,pages_length:%#018lx\n",
               memory_management_struct.pages_struct,
               memory_management_struct.pages_size,
               memory_management_struct.pages_length);

  color_printk(ORANGE, BLACK,
               "zones_struct:%#018lx,zones_size:%#018lx,zones_length:%#018lx\n",
               memory_management_struct.zones_struct,
               memory_management_struct.zones_size,
               memory_management_struct.zones_length);

  ZONE_DMA_INDEX = 0;
  ZONE_NORMAL_INDEX = 0;

  for (i = 0; i < memory_management_struct.zones_size;
       i++) // need rewrite in the future
  {
    struct zone *z = memory_management_struct.zones_struct + i;
    color_printk(ORANGE, BLACK,
                 "zone_start_address:%#018lx,zone_end_address:%#018lx,zone_"
                 "length:%#018lx,pages_group:%#018lx,pages_length:%#018lx\n",
                 z->zone_start_address, z->zone_end_address, z->zone_length,
                 z->pages_group, z->pages_length);
    if (z->zone_start_address == 0x100000000)
      ZONE_UNMAPED_INDEX = i;
  }
  memory_management_struct.end_of_struct =
      (unsigned long)((unsigned long)memory_management_struct.zones_struct +
                      memory_management_struct.zones_length +
                      sizeof(long) * 32) &
      (~(sizeof(long) -
         1)); ////need a blank to separate memory_management_struct
  color_printk(
      ORANGE, BLACK,
      "start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,end_brk:%#018lx,"
      "end_of_struct:%#018lx\n",
      memory_management_struct.start_code, memory_management_struct.end_code,
      memory_management_struct.end_data, memory_management_struct.end_brk,
      memory_management_struct.end_of_struct);

  i = VIRT_TO_PHY(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;

  for (j = 0; j <= i; j++) {
    page_init(memory_management_struct.pages_struct + j,
              PG_PTABLE_MAPED | PG_KERNEL_INIT | PG_ACTIVE | PG_KERNEL);
  }
  global_cr3 = get_gdt();

  color_printk(INDIGO, BLACK, "Global_CR3\t:%#018lx\n", global_cr3);
  color_printk(INDIGO, BLACK, "*Global_CR3\t:%#018lx\n",
               *PHY_TO_VIRT(global_cr3) & (~0xff));
  color_printk(PURPLE, BLACK, "**global_cr3\t:%#018lx\n",
               *PHY_TO_VIRT(*PHY_TO_VIRT(global_cr3) & (~0xff)) & (~0xff));

  // for (i = 0; i < 10; i++)
  //   *(PHY_TO_VIRT(global_cr3) + i) = 0UL;

  FLUSH_TLB();
}

/**
 * alloc page
 * @param zone_select
 * @param number
 * @param page_flags
 * @return
 */
struct page *alloc_pages(int zone_select, int number,
                         unsigned long page_flags) {
  int i;
  unsigned long page = 0;

  int zone_start = 0;
  int zone_end = 0;

  switch (zone_select) {
  case ZONE_DMA:
    zone_start = 0;
    zone_end = ZONE_DMA_INDEX;
    break;
  case ZONE_NOMAL:
    zone_start = ZONE_DMA_INDEX;
    zone_end = ZONE_NORMAL_INDEX;
    break;
  case ZONE_UNMAPED:
    zone_start = ZONE_UNMAPED_INDEX;
    zone_end = memory_management_struct.zones_size - 1;
    break;
  default:
    color_printk(RED, BLACK, "alloc_pages error zone_select ZONE_DMA_INDE\n");
    return NULL;
    break;
  }

  for (i = zone_start; i <= zone_end; i++) {
    struct zone *z;
    unsigned long j;
    unsigned long start, end, length;
    unsigned long tmp;

    if ((memory_management_struct.zones_struct + i)->page_free_count < number) {
      continue;
    }

    z = memory_management_struct.zones_struct + i;
    start = z->zone_start_address >> PAGE_2M_SHIFT;
    end = z->zone_end_address >> PAGE_2M_SHIFT;
    length = z->zone_length >> PAGE_2M_SHIFT;

    tmp = 64 - start % 64;

    for (j = start; j <= end; j += j % 64 ? tmp : 64) {
      unsigned long *p = memory_management_struct.bits_map + (j >> 6);
      unsigned long shift = j % 64;
      unsigned long k;

      for (k = shift; k < 64 - shift; k++) {
        if (!(((*p >> k) | (*(p + 1) << (64 - k))) &
              (number == 64 ? 0xffffffffffffffffUL : ((1UL << number) - 1)))) {
          unsigned long l;
          page = j + k - 1;
          for (l = 0; l < number; l++) {
            struct page *x = memory_management_struct.pages_struct + page + l;
            page_init(x, page_flags);
          }
          goto find_free_pages;
        }
      }
    }
  }
  return NULL;
find_free_pages:
  return (struct page *)(memory_management_struct.pages_struct + page);
}
