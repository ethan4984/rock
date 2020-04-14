#ifndef STIVALE_DEFS_H
#define STIVALE_DEFS_H
#include <stdint.h>

#define STIVALE_MEMORY_AVAILABLE              1
#define STIVALE_MEMORY_RESERVED               2
#define STIVALE_MEMORY_ACPI_RECLAIMABLE       3
#define STIVALE_MEMORY_NVS                    4
#define STIVALE_MEMORY_BADRAM                 5

struct stivale_module {
    uint64_t begin;
    uint64_t end;
    char     string[128];
} __attribute__((packed));

typedef struct {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t unused;
} __attribute__((packed)) e820_entry_t;

typedef struct stivale_struct {
    char    *cmdline;
    uint64_t memory_map_addr;
    uint64_t memory_map_entries;
    uint64_t framebuffer_addr;
    uint16_t framebuffer_pitch;
    uint16_t framebuffer_width;
    uint16_t framebuffer_height;
    uint16_t framebuffer_bpp;
    uint64_t rsdp;
    uint64_t module_count;
    struct stivale_module modules[];
} __attribute__((packed)) stivale_info_t;

#endif
