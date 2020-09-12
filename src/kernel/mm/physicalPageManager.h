#pragma once

#include <kernel/stivale.h>
#include <stdint.h>

namespace kernel {
    
class physicalPageManager_t {
public:
    void init(stivaleInfo_t *stivaleInfo);

    uint64_t alloc(uint64_t cnt);

    void free(uint64_t base, uint64_t count);

private:
    uint8_t *bitmap; 

    uint64_t totalDetectedMemory = 0;

    void allocateRegion(uint64_t start, uint64_t length);

    void freeRegion(uint64_t start, uint64_t length);

    int64_t firstFreePage();
};

inline physicalPageManager_t physicalPageManager;

}
