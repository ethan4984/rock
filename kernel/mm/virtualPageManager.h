#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGESIZE 0x1000
#define USR_PT_FLAGS (1 << 7) | 0x3
#define USR_PD_FLAGS 0x3

#define KERNEL_HIGH_VMA 0xffffffff80000000
#define HIGH_VMA 0xffff800000000000

#define ERROR 0xffffffffffffffff

namespace kernel {

class pdEntry_t {
public:
    pdEntry_t(uint64_t pml4, uint64_t pdFlags, uint64_t ptFlags, uint64_t hashIdentifier);

    pdEntry_t() { }

    uint64_t findPageMap(uint64_t identifier);

    void initPageMap(uint64_t index);

protected:
    uint64_t pml4;
    uint64_t pdFlags;
    uint64_t ptFlags;

    static uint32_t *hash;
    static uint32_t hashSize;
};

class virtualPageManager_t : protected pdEntry_t {
public:
    void init();

    void newUserMap(uint64_t pageCnt);

    void fork(uint64_t pageMapIndex);

private:
    static pdEntry_t *pdEntries;
};

inline virtualPageManager_t virtualPageManager;

}
