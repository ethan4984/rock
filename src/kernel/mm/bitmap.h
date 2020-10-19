#pragma once 

#define BLOCK_SIZE 32

#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <stdint.h>

namespace mm {

struct allocation_t {
    uint32_t blkCnt;
    uint32_t blkIdx;
};

class bitmapHeap { 
public:
    void init(uint64_t pageCnt);

    void *alloc(uint64_t size);

    uint64_t free(void *addr);

    void *realloc(void *addr, uint64_t size);
    
    uint64_t start, bitmapSize, allocationSize;
protected:
    uint8_t *bitmap;

    allocation_t *allocations;

    int64_t firstFreeSlot();

    int64_t firstFreeAllocationSlot();

    char lock = 0;
};

}
