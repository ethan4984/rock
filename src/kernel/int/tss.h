#pragma once

#include <stdint.h>

namespace kernel {

struct tss_t {
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
} __attribute__((packed));

class tssMain_t {
public:
    void init(); 

    void newTss(uint64_t rsp0);
    
    tss_t *tss;
};

inline tssMain_t tssMain;

}
