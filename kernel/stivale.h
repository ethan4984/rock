#ifndef STIVALE_H_
#define STIVALE_H_

#include <stddef.h>
#include <stdint.h>

struct stivale_module_t {
    uint64_t begin;
    uint64_t end;
    char string[128];
} __attribute__((packed));

struct stivale_mmap {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t unused;
} __attribute__((packed));

struct stivale {
    char *cmdline;
    uint64_t mmap_addr;
    uint64_t mmap_cnt;
    uint64_t fb_addr;
    uint16_t fb_pitch;
    uint16_t fb_width;
    uint16_t fb_height;
    uint16_t fb_bpp;
    uint64_t rsdp;
    uint64_t module_cnt;
    struct stivale_module_t modules[];
} __attribute__((packed));

#endif
