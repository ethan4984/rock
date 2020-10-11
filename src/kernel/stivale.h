#pragma once

#include <stdint.h>

struct [[gnu::packed]] stivaleModule {
    uint64_t begin;
    uint64_t end;
    char string[128];
};

struct [[gnu::packed]] stivaleMMAPentry_t {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t unused;
};

struct [[gnu::packed]] stivaleInfo_t {
    char *cmdline;
    uint64_t memoryMapAddr;
    uint64_t memoryMapEntries;
    uint64_t framebufferAddr;
    uint16_t framebufferPitch;
    uint16_t framebufferWidth;
    uint16_t framebufferHeight;
    uint16_t framebufferBpp;
    uint64_t rsdp;
    uint64_t moduleCount;
    struct stivaleModule modules[];
};
