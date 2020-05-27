#pragma once

#include <Kernel/stivale.h>
#include <stdint.h>

/* contains the addressing spec for usable ram */
typedef struct {
    uint64_t type;
    uint64_t mmapEntryNum; 
    uint64_t base; /* base address */
    uint64_t limit; /* size in bytes of the regin */
} memoryRegion;

class E820map {
    public:
        E820map(stivaleInfo_t *bootInfo);

        void getNextUseableMem(memoryRegion *memRegion);

        void getNthMmap(memoryRegion *memRegion, uint64_t num);

        uint64_t totalMem = 0;
    private:
        E820Entry_t *mmap;

        stivaleInfo_t *stivaleInfo;
};
