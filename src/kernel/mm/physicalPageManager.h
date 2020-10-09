#pragma once

#include <kernel/stivale.h>
#include <stdint.h>

namespace pmm { 

void init(stivaleInfo_t *stivaleInfo);

uint64_t alloc(uint64_t cnt);

uint64_t calloc(uint64_t cnt);

void free(uint64_t base, uint64_t cnt);

}
