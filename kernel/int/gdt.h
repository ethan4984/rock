#ifndef GDT_H_
#define GDT_H_

#include <memutils.h>
#include <cpu.h>

extern void lgdt(uint64_t gdt_addr);
extern void ltr(uint64_t selector);

struct memory_segment {
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity; 
    uint8_t base_high;
} __attribute__((packed));

struct tss_segment {
    uint16_t length;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t flags1;
    uint8_t flags2; 
    uint8_t base_high;
    uint32_t base_high32;
    uint32_t reserved;
} __attribute__((packed));

struct tss {
    uint32_t reserved;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint32_t reserved1;
    uint32_t reserved2;
    uint64_t ist1;
    uint64_t ist2; 
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t IOPB;
} __attribute__((packed));

struct gdtr {
    uint16_t limit;
    uint64_t offset;
} __attribute__((packed));

struct gdt {
    struct memory_segment mem_segments[5];
    struct tss_segment tss_segments[32];
    struct gdtr gdtr;
} __attribute__((packed));

void gdt_init();
void add_tss_segment();
void create_generic_tss();

#endif
