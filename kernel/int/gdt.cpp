#include <int/gdt.hpp>

namespace x86 {

static gdt system_gdt;

void gdt_init() {
    system_gdt.seg_desc[1].limit = 0;
    system_gdt.seg_desc[1].base_low = 0;
    system_gdt.seg_desc[1].base_mid = 0;
    system_gdt.seg_desc[1].access = 0b10011010;
    system_gdt.seg_desc[1].granularity = 0b00100000;
    system_gdt.seg_desc[1].base_high = 0;

    // data 64
    system_gdt.seg_desc[2].limit = 0;
    system_gdt.seg_desc[2].base_low = 0;
    system_gdt.seg_desc[2].base_mid = 0;
    system_gdt.seg_desc[2].access = 0b10010110;
    system_gdt.seg_desc[2].granularity = 0;
    system_gdt.seg_desc[2].base_high = 0;

    // user data 64
    system_gdt.seg_desc[3].limit = 0;
    system_gdt.seg_desc[3].base_low = 0;
    system_gdt.seg_desc[3].base_mid = 0;
    system_gdt.seg_desc[3].access = 0b11110010;
    system_gdt.seg_desc[3].granularity = 0;
    system_gdt.seg_desc[3].base_high = 0;

    // user code 64
    system_gdt.seg_desc[4].limit = 0;
    system_gdt.seg_desc[4].base_low = 0;
    system_gdt.seg_desc[4].base_mid = 0;
    system_gdt.seg_desc[4].access = 0b11111010;
    system_gdt.seg_desc[4].granularity = 0b00100000;

    system_gdt.gdt_reg.offset = (uint64_t)&system_gdt.seg_desc;
    system_gdt.gdt_reg.limit = sizeof(struct segment_descriptor) * 5 - 1;

    lgdt((size_t)&system_gdt.gdt_reg);
}

}
