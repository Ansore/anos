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

// page table attribute

// bit63 execution disable
#define PAGE_XD (unsigned long)0x1000000000000000
// bit12 page attribute table
#define PAGE_PAT (unsigned long)0x1000
// bit8 global page 1:global, 0:part
#define PAGE_GLOBAL (unsigned long)0x100
// bit7 page size  1:big page, 0:small page
#define PAGE_PS (unsigned long)0x0080
// bit6 dirty 1:dirty, 0:clean
#define PAGE_DIRTY (unsigned long)0x0040
// bit5 accessed 1:visited, 0:unvisited
#define PAGE_ACCESSED (unsigned long)0x0020
// bit4 page level cache disable
#define PAGE_PCD (unsigned long)0x0010
// bit3 page level write through
#define PAGE_PWT (unsigned long)0x0008
// bit2 user supervisor 1:user and supervisor, 0:supervisor
#define PAGE_U_S (unsigned long)0x0004
// bit1 read write 1:read and write, 0:read
#define PAGE_R_W (unsigned long)0x0002
// bit0 present 1:present, 0:no present
#define PAGE_PRESENT (unsigned long)0x0001

// 1,0
#define PAGE_KERNEL_GDT (PAGE_R_W | PAGE_PRESENT)
// 1,0
#define PAGE_KERNEL_DIR (PAGE_R_W | PAGE_PRESENT)
// 7,1,0
#define PAGE_KERNEL_PAGE (PAGE_PS | PAGE_R_W | PAGE_PRESENT)
// 2,1,0
#define PAGE_USER_DIR (PAGE_U_S | PAGE_R_W | PAGE_PRESENT)
// 7,2,1,0
#define PAGE_USER_PAGE (PAGE_PS | PAGE_U_S | PAGE_R_W | PAGE_PRESENT)

#define mk_mpl4t(addr, attr) ((unsigned long)(addr) | (unsigned long)(attr))
#define set_mpl4t(mpl4tptr, mpl4tval) (*(mpl4tptr) = (mpl4tval))

#define mk_pdpt(addr, attr) ((unsigned long)(addr) | (unsigned long)(attr))
#define set_pdpt(pdptptr, pdptval) (*(pdptptr) = (pdptval))

#define mk_pdt(addr, attr) ((unsigned long)(addr) | (unsigned long)(attr))
#define set_pdt(pdtptr, pdtval) (*(pdtptr) = (pdtval))

#define mk_pt(addr, attr) ((unsigned long)(addr) | (unsigned long)(attr))
#define set_pt(ptptr, ptval) (*(ptptr) = (ptval))

typedef struct {
  unsigned long pmlt4;
} pml4t_t;

typedef struct {
  unsigned long pdpt;
} pdpt_t;

typedef struct {
  unsigned long pdt;
} pdt_t;

typedef struct {
  unsigned long pt;
} pt_t;

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
