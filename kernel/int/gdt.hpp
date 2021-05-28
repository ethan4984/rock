#ifndef GDT_HPP_
#define GDT_HPP_

#include <cstdint>
#include <cstddef>

namespace x86 {

extern "C" void lgdt(size_t addr);
extern "C" void ltr(size_t selector);

constexpr size_t kernel64_cs = 0x8;
constexpr size_t kernel64_ds = 0x10;
constexpr size_t user64_cs = 0x23;
constexpr size_t user64_ds = 0x1b;

struct [[gnu::packed]] segment_descriptor {
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity; 
    uint8_t base_high;
};

struct [[gnu::packed]] tss_descriptor {
    uint16_t length;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t flags1;
    uint8_t flags2; 
    uint8_t base_high;
    uint32_t base_high32;
    uint32_t reserved;
};

struct [[gnu::packed]] gdtr {
    uint16_t limit;
    uint64_t offset;
};

struct gdt {
    segment_descriptor seg_desc[5];
    gdtr gdt_reg;
};

void gdt_init();

}

#endif
