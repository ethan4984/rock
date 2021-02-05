#ifndef PMM_H_
#define PMM_H_

#include <stivale.h>
#include <memutils.h>

extern size_t total_physical_mem;

void pmm_init(stivale_t *stivale);
size_t pmm_alloc(uint64_t cnt);
size_t pmm_calloc(uint64_t cnt);
void pmm_free(uint64_t base, uint64_t cnt);

#endif 
