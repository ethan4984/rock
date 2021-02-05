#include <int/gdt.h>
#include <bitmap.h>

static gdt_t gdt;
static int tss_cnt = 0;

void gdt_init() {
    // code 64
    gdt.mem_segments[1].limit = 0;
    gdt.mem_segments[1].base_low = 0;
    gdt.mem_segments[1].base_mid = 0;
    gdt.mem_segments[1].access = 0b10011010;
    gdt.mem_segments[1].granularity = 0b00100000;
    gdt.mem_segments[1].base_high = 0;

    // data 64
    gdt.mem_segments[2].limit = 0;
    gdt.mem_segments[2].base_low = 0;
    gdt.mem_segments[2].base_mid = 0;
    gdt.mem_segments[2].access = 0b10010110;
    gdt.mem_segments[2].granularity = 0;
    gdt.mem_segments[2].base_high = 0;

    // user data 64
    gdt.mem_segments[3].limit = 0;
    gdt.mem_segments[3].base_low = 0;
    gdt.mem_segments[3].base_mid = 0;
    gdt.mem_segments[3].access = 0b11110010;
    gdt.mem_segments[3].granularity = 0;
    gdt.mem_segments[3].base_high = 0;

    // user code 64
    gdt.mem_segments[4].limit = 0;
    gdt.mem_segments[4].base_low = 0;
    gdt.mem_segments[4].base_mid = 0;
    gdt.mem_segments[4].access = 0b11111010;
    gdt.mem_segments[4].granularity = 0b00100000;

    gdt.gdtr.offset = (uint64_t)&gdt.mem_segments;
    gdt.gdtr.limit = sizeof(memory_segment_t) * 5 - 1;

    lgdt((uint64_t)&gdt.gdtr);
}

void gdt_tss_segment(uint64_t tss_addr) {
    gdt.tss_segments[tss_cnt].length = 104;
    gdt.tss_segments[tss_cnt].base_low = (uint16_t)tss_addr;
    gdt.tss_segments[tss_cnt].base_mid = (uint8_t)(tss_addr >> 16);
    gdt.tss_segments[tss_cnt].flags1 = 0b10001001;
    gdt.tss_segments[tss_cnt].flags2 = 0;
    gdt.tss_segments[tss_cnt].base_high = (uint8_t)(tss_addr >> 24);
    gdt.tss_segments[tss_cnt++].base_high32 = (uint32_t)(tss_addr >> 32);

    gdt.gdtr.limit += sizeof(tss_segment_t);
    lgdt((uint64_t)&gdt.gdtr); 

    ltr(gdt.gdtr.limit - sizeof(tss_segment_t) + 1);
}

void create_generic_tss() {
    tss_t *tss = kmalloc(sizeof(tss_t));

    tss->rsp0 = pmm_alloc(2) + 0x2000 + HIGH_VMA;
    tss->rsp1 = pmm_alloc(2) + 0x2000 + HIGH_VMA;
    tss->rsp2 = pmm_alloc(2) + 0x2000 + HIGH_VMA;

    gdt_tss_segment((uint64_t)tss);
}
