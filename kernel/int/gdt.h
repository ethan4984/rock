#pragma once

#include <stdint.h>

namespace kernel {

extern "C" void lgdt(uint64_t gdtAddr, uint64_t tssAddr);

struct gdtEntry_t {
    uint16_t limit;
    uint16_t baseLow;
    uint8_t baseMid;
    uint8_t access;
    uint8_t granularity; 
    uint8_t baseHigh;
} __attribute__((packed));

struct gdtTSS_t {
    uint16_t length;
    uint16_t baseLow;
    uint8_t baseMid;
    uint8_t flags1;
    uint8_t flags2; 
    uint8_t baseHigh;
    uint32_t baseHigh32;
    uint32_t reserved; 
} __attribute__((packed));

struct gdtr_t {
    uint16_t limit;
    uint64_t offset;
} __attribute__((packed));

struct gdtCore_t {
    gdtEntry_t gdtEntries[5];
    gdtTSS_t tss;
    gdtr_t gdtr;
} __attribute__((packed));

class gdt_t {
public:
    void gdtInit();

    void initCore(uint64_t core, uint64_t tssAddr);
private:
    gdtCore_t *gdtCores;
};

inline gdt_t gdt;

}
