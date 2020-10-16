#pragma once

#include <kernel/mm/bitmap.h>

void *operator new(uint64_t size);

void *operator new[](uint64_t size);

void operator delete(void *addr, uint64_t size);

void operator delete(void *addr);

inline mm::bitmapHeap kheap;
