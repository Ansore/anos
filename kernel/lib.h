#ifndef __LIB_H__
#define __LIB_H__

#define NULL 0

#define sti() __asm__ __volatile__("sti \n\t" ::: "memory")
#define cli() __asm__ __volatile__("cli \n\t" ::: "memory")
#define nop() __asm__ __volatile__("nop \n\t")
#define io_mfence() __asm__ __volatile__("mfence \n\t" ::: "memory")

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
