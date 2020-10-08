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

#define GET_PMLX_FLAGS(pmlX) ((pmlX) & 0xfff)
#define GET_PMLX_ADDR(pmlX) ((pmlX) & ~(0xfff))

namespace vmm {

struct mapping { 
    uint64_t *pml4 = NULL;

    void init();
    void exit();
    void copy(mapping m1); 

    static void mappingsInit();
    static void addMapping(mapping newMapping);
    static mapping *mappings;
    static uint64_t mappingCnt;
};

void init();

void map(uint64_t physicalAddr, uint64_t virtualAddress, uint64_t flags, uint64_t flags1);

void unmap(uint64_t virtualAddress, uint64_t flags);

uint64_t grabPML4();

void tlbFlush();

}
