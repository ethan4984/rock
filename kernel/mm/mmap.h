#ifndef MMAP_H_
#define MMAP_H_

#include <stdint.h>

#define MMAP_MIN_ADDR 0x10000

enum {
    MAP_SHARED,
    MAP_PRIVATE,
    MAP_FIXED
};

void *mmap(void *addr, uint64_t size, int prot, int flags, int fd, int64_t off);

int munmap(void *addr, uint64_t size);

#endif
