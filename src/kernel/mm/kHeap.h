#pragma once

#define BLOCKSIZE 32

#include <stdint.h>

void *operator new(uint64_t size);

void *operator new[](uint64_t size);

void operator delete(void *addr, uint64_t size);

void operator delete(void *addr);

namespace kernel {

struct allocation_t {
    uint16_t block;
    uint32_t count;
};

class kheap_t {
public:
    void init();

    void *kmalloc(uint64_t size);

    uint64_t kfree(void *addr);

    void *krealloc(void *addr, uint64_t size);
private:
    uint64_t heapBegin;

    int64_t bitmapSize, allocationSize;

    uint8_t *bitmap;

    allocation_t *allocation;

    int64_t firstFreeSlot();

    int64_t firstFreeAllocationSlot();
};

inline kheap_t kheap;

}
