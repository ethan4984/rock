#pragma once

#include <stdint.h>

struct [[gnu::packed]] tss_t {
    uint32_t reserved = 0;
    uint64_t rsp0 = 0;
    uint64_t rsp1 = 0;
    uint64_t rsp2 = 0;
    uint32_t reserved1 = 0;
    uint32_t reserved2 = 0;
    uint64_t ist1 = 0;
    uint64_t ist2 = 0; 
    uint64_t ist3 = 0;
    uint64_t ist4 = 0;
    uint64_t ist5 = 0;
    uint64_t ist6 = 0;
    uint64_t ist7 = 0;
    uint64_t reserved3 = 0;
    uint16_t reserved4 = 0;
    uint16_t IOPB = 0;
};

namespace tss {

void init();

tss_t *create();

}
