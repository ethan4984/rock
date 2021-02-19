#ifndef PMM_H_
#define PMM_H_

#include <stivale.h>
#include <memutils.h>

extern size_t total_physical_mem;

void pmm_init(struct stivale *stivale);
size_t pmm_alloc(size_t cnt);
size_t pmm_calloc(size_t cnt);
void pmm_free(size_t base, size_t cnt);

#endif
