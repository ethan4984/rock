#pragma once

#include <stdint.h>
#include <stddef.h>

#define PAGESIZE 0x1000

#define USR_PT_FLAGS (1 << 2) | (1 << 7) | 0x3 
#define USR_PD_FLAGS (1 << 2) | 0x3

#define KERNEL_PT_FLAGS (1 << 2) | (1 << 7) | 0x3
#define KERNEL_PD_FLAGS (1 << 2) | 0x3

#define KERNEL_HIGH_VMA 0xffffffff80000000
#define HIGH_VMA 0xffff800000000000

#define ERROR 0xffffffffffffffff

struct pageMap_t {
    int64_t pml4Index; 
    int64_t pml3Index;
    int64_t pml2Index;
    int64_t pml1Index;
};

class pdEntry_t {
public:
    pdEntry_t(uint64_t pml4, uint64_t pdFlags, uint64_t ptFlags, uint64_t hashIdentifier);

    pdEntry_t() { }

    void initPageMap();
    
    uint64_t pml4;

    uint64_t pdFlags;

    uint64_t ptFlags;
};

class virtualPageManager_t : protected pdEntry_t {
public:
    void init();

    uint64_t newUserMap(uint64_t pageCnt);

    void map(uint64_t base, uint64_t physicalBase, uint64_t cnt, uint64_t flags);

    void unmap(uint64_t base, uint64_t cnt, uint64_t flags);

    uint64_t grabPML4();

    uint64_t firstFreeSlot();

    void initAddressSpace(uint64_t index);

private:
    pageMap_t getIndexes(uint64_t base, uint64_t flags);

    void tlbFlush();

    uint64_t entryCnt = 0x1000;

    static pdEntry_t *pdEntries;
};

inline virtualPageManager_t virtualPageManager;
