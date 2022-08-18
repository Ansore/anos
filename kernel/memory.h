#ifndef __MEMORY_H__
#define __MEMORY_H__

// 8 bytes per cell
#include "lib.h"
#define PTRS_PER_PAGE 512

#define PAGE_OFFSET ((unsigned long)0xffff800000000000)

#define PAGE_GDT_SHIFT 39
#define PAGE_1G_SHIFT 30
#define PAGE_2M_SHIFT 21
#define PAGE_4K_SHIFT 12

#define PAGE_2M_SIZE (1UL << PAGE_2M_SHIFT)
#define PAGE_4K_SIZE (1UL << PAGE_4K_SHIFT)

#define PAGE_2M_MASK (~(PAGE_2M_SIZE - 1))
#define PAGE_4K_MASK (~(PAGE_4K_SIZE - 1))

#define PAGE_2M_ALIGN(addr)                                                    \
  (((unsigned long)(addr) + PAGE_2M_SIZE - 1) & PAGE_2M_MASK)
#define PAGE_4K_ALIGN(addr)                                                    \
  (((unsigned long)(addr) + PAGE_4K_SIZE - 1) & PAGE_4K_MASK)

#define VIRT_TO_PHY(addr) ((unsigned long)(addr)-PAGE_OFFSET)
#define PHY_TO_VIRT(addr)                                                      \
  ((unsigned long *)((unsigned long)(addr) + PAGE_OFFSET))

#define ZONE_DMA (1 << 0)
#define ZONE_NOMAL (1 << 1)
#define ZONE_UNMAPED (1 << 2)

#define PG_PTABLE_MAPED (1 << 0)
#define PG_KERNEL_INIT (1 << 1)
#define PG_REFRENCED (1 << 2)
#define PG_DIRTY (1 << 3)
#define PG_ACTIVE (1 << 4)
#define PG_UP_TO_DATE (1 << 5)
#define PG_DEVICE (1 << 6)
#define PG_KERNEL (1 << 7)
#define PG_K_SHARE_TO_U (1 << 8)
#define PG_SLAB (1 << 9)

#define MAX_NR_ZONES 10 // max zone

struct E820 {
  unsigned long address;
  unsigned long length;
  unsigned int type;
} __attribute__((packed));

struct global_memory_descriptor {
  struct E820 e820[32];
  unsigned long e820_length;

  unsigned long *bits_map;
  unsigned long bits_size;
  unsigned long bits_length;

  struct page *pages_struct;
  unsigned long pages_size;
  unsigned long pages_length;

  struct zone *zones_struct;
  unsigned long zones_size;
  unsigned long zones_length;

  unsigned long start_code, end_code, end_data, end_brk;

  unsigned long end_of_struct;
};

struct zone {
  struct page *pages_group;
  unsigned long pages_length;
  unsigned long zone_start_address;
  unsigned long zone_end_address;
  unsigned long zone_length;
  unsigned long attribute;

  struct global_memory_descriptor *gmd_struct;

  unsigned long page_using_count;
  unsigned long page_free_count;
  unsigned long total_pages_link;
};

struct page {
  struct zone *zone_struct;
  unsigned long phy_address;
  unsigned long attribute;
  unsigned long reference_count;
  unsigned long age;
};

extern struct global_memory_descriptor memory_management_struct;
// each zone index

int ZONE_DMA_INDEX = 0;
int ZONE_NORMAL_INDEX = 0;  // low 1GB RAM ,was mapped in pagetable
int ZONE_UNMAPED_INDEX = 0; // above 1GB RAM,unmapped in pagetable

unsigned long *global_cr3 = NULL;

#define FLUSH_TLB_ONE(addr)                                                    \
  __asm__ __volatile__("invlpg (%0) \n\t" : : "r"(addr) : "memory");

#define FLUSH_TLB()                                                            \
  do {                                                                         \
    unsigned long tmpreg;                                                      \
    __asm__ __volatile__("movq %%cr3, %0 \n\t"                                 \
                         "movq %0, %%cr3 \n\t"                                 \
                         : "=r"(tmpreg)                                        \
                         :                                                     \
                         : "memory");                                          \
  } while (0)

static inline unsigned long *get_gdt() {
  unsigned long *tmp;
  __asm__ __volatile__("movq %%cr3, %0 \n\t" : "=r"(tmp) : : "memory");
  return tmp;
}

void memory_init();
struct page *alloc_pages(int zone_select, int number, unsigned long page_flags);

#endif // !__MEMORY_H__
