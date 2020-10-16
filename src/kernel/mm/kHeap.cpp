#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <lib/output.h>

#include <stddef.h>

void *operator new(uint64_t size) {
    return kheap.alloc(size);
}

void *operator new[](uint64_t size) {
    return kheap.alloc(size);
}

void operator delete(void *addr) {
    kheap.free(addr);
}

void operator delete[](void *addr) {
    kheap.free(addr);
}

void operator delete[](void *addr, uint64_t size) {
    kheap.free(addr);
}

void operator delete(void *addr, uint64_t size) {
    kheap.free(addr);
}
