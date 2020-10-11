#pragma once

#include <stdint.h>

extern "C" void lgdt(uint64_t gdtAddr, uint64_t tssAddr);

struct [[gnu::packed]] gdtEntry_t {
    uint16_t limit;
    uint16_t baseLow;
    uint8_t baseMid;
    uint8_t access;
    uint8_t granularity; 
    uint8_t baseHigh;
};

struct [[gnu::packed]] gdtTSS_t {
    uint16_t length;
    uint16_t baseLow;
    uint8_t baseMid;
    uint8_t flags1;
    uint8_t flags2; 
    uint8_t baseHigh;
    uint32_t baseHigh32;
    uint32_t reserved; 
};

struct [[gnu::packed]] gdtr_t {
    uint16_t limit;
    uint64_t offset;
};

struct [[gnu::packed]] gdtCore_t {
    gdtEntry_t gdtEntries[5];
    gdtTSS_t tss;
    gdtr_t gdtr;
};

namespace gdt {

void init();

void initCore(uint64_t core, uint64_t tssAddr);

inline gdtCore_t *gdtCores;

}
