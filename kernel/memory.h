#ifndef __MEMORY_H__
#define __MEMORY_H__

struct Memory_E820_Formate {
  unsigned int address1;
  unsigned int address2;
  unsigned int length1;
  unsigned int length2;
  unsigned int type;
};

void memory_init();

#endif // !__MEMORY_H__
