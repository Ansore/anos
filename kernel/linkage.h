#ifndef __LINKAGE_H
#define __LINKAGE_H

#define L1_CACHE_BYTES 32

#define asmlinkage __attribute__((regparm(0)))

#define ____cacheline_aligned __attribute__((__ligned__(L1_CACHE_BYTES)))

#define SYMBOL_NAME(X) X

#define SYMBOL_NAME_STR(X) #X

#define SYMBOL_NAME_LABLE(X) X##:

#define ENTRY(name) \
        .global SYMBOL_NAME(name); \
        SYMBOL_NAME_LABLE(name)

#endif // !__LINKAGE_H
