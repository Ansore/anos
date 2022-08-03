#ifndef __LIB_H__
#define __LIB_H__

#define NULL 0

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

#endif // !__LIB_H__
