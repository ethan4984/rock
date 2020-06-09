#pragma once

#include <Kernel/mm/memoryParse.h>

void initPageHandler(stivaleInfo_t *bootInfo);

void *physicalPageAlloc(uint64_t count);

void *physicalPageRealloc(void *ptr, uint64_t size);

void physicalPageFree(void* address);

void allocateRegion(uint64_t start, uint64_t end);

uint64_t roundUp(uint64_t divided, uint64_t divisor);

void *getE820();
