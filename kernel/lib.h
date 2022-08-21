#ifndef __LIB_H__
#define __LIB_H__

#define NULL 0

#define container_of(ptr, type, member)                                        \
  ({                                                                           \
    typeof(((type *)0)->member) *p = (ptr);                                    \
    (type *)((unsigned long)p - (unsigned long)&(((type *)0)->member));        \
  })

#define sti() __asm__ __volatile__("sti \n\t" ::: "memory")
#define cli() __asm__ __volatile__("cli \n\t" ::: "memory")
#define nop() __asm__ __volatile__("nop \n\t")
#define io_mfence() __asm__ __volatile__("mfence \n\t" ::: "memory")

struct list {
  struct list *prev;
  struct list *next;
};

static inline void list_init(struct list *list) {
  list->prev = list;
  list->next = list;
}

static inline void list_add_to_behind(struct list *entry, struct list *newe) {
  newe->next = entry->next;
  newe->prev = entry;
  newe->next->prev = newe;
  entry->next = newe;
}

static inline void list_add_to_before(struct list *entry, struct list *newe) {
  newe->next = entry;
  entry->prev->next = newe;
  newe->prev = entry->prev;
  entry->prev = newe;
}

static inline void list_del(struct list *entry) {
  entry->next->prev = entry->prev;
  entry->prev->next = entry->next;
}

static inline long list_is_empty(struct list *entry) {
  if (entry == entry->next && entry->prev == entry) {
    return 1;
  } else {
    return 0;
  }
}

static inline struct list *list_prev(struct list *entry) {
  if (entry->prev != NULL) {
    return entry->prev;
  } else {
    return NULL;
  }
}

static inline struct list *list_next(struct list *entry) {
  if (entry->next != NULL) {
    return entry->next;
  } else {
    return NULL;
  }
}

/**
 * get string length
 * @param str the point of string
 * @return string length
 */
static inline int strlen(char *str) {
  register int __res;
  __asm__ __volatile__("cld \n\t"
                       "repne \n\t"
                       "scasb \n\t"
                       "notl %0 \n\t"
                       "decl %0 \n\t"
                       : "=c"(__res)
                       : "D"(str), "a"(0), "0"(0xffffffff));
  return __res;
}

static inline void *memset(void *address, unsigned char c, long count) {
  int d0, d1;
  unsigned long tmp = c * 0x0101010101010101UL;
  __asm__ __volatile__("cld \n\t"
                       "rep \n\t"
                       "stosq \n\t"
                       "testb $4, %b3 \n\t"
                       "je 1f \n\t"
                       "stosl \n\t"
                       "1:\ttestb $2, %b3 \n\t"
                       "je 2f\n\t"
                       "stosw \n\t"
                       "2:\ttestb $1, %b3 \n\t"
                       "je 3f \n\t"
                       "stosb \n\t"
                       "3: \n\t"
                       : "=&c"(d0), "=&D"(d1)
                       : "a"(tmp), "q"(count), "0"(count / 8), "1"(address)
                       : "memory");
  return address;
}

static inline void *memcpy(void *From, void *To, long Num) {
  int d0, d1, d2;
  __asm__ __volatile__("cld	\n\t"
                       "rep	\n\t"
                       "movsq	\n\t"
                       "testb	$4,%b4	\n\t"
                       "je	1f	\n\t"
                       "movsl	\n\t"
                       "1:\ttestb	$2,%b4	\n\t"
                       "je	2f	\n\t"
                       "movsw	\n\t"
                       "2:\ttestb	$1,%b4	\n\t"
                       "je	3f	\n\t"
                       "movsb	\n\t"
                       "3:	\n\t"
                       : "=&c"(d0), "=&D"(d1), "=&S"(d2)
                       : "0"(Num / 8), "q"(Num), "1"(To), "2"(From)
                       : "memory");
  return To;
}

static inline unsigned int io_in8(unsigned short port) {
  unsigned char ret = 0;
  __asm__ __volatile__("inb %%dx, %0 \n\t"
                       "mfence \n\t"
                       : "=a"(ret)
                       : "d"(port)
                       : "memory");
  return ret;
}

static inline unsigned int io_in32(unsigned short port) {
  unsigned int ret = 0;
  __asm__ __volatile__("inl %%dx, %0 \n\t"
                       "mfence \n\t"
                       : "=a"(ret)
                       : "d"(port)
                       : "memory");
  return ret;
}

static inline void io_out8(unsigned short port, unsigned char value) {
  __asm__ __volatile__("outb %0, %%dx \n\t"
                       "mfence \n\t"
                       :
                       : "a"(value), "d"(port)
                       : "memory");
}

static inline void io_out32(unsigned short port, unsigned int value) {
  __asm__ __volatile__("outl %0, %%dx \n\t"
                       "mfence \n\t"
                       :
                       : "a"(value), "d"(port)
                       : "memory");
}

#endif // !__LIB_H__
