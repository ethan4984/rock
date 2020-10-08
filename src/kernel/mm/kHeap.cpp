#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <lib/output.h>

#include <stddef.h>

void *operator new(uint64_t size) {
    return kheap.kmalloc(size);
}

void *operator new[](uint64_t size) {
    return kheap.kmalloc(size);
}

void operator delete(void *addr) {
    kheap.kfree(addr);
}

void operator delete[](void *addr) {
    kheap.kfree(addr);
}

namespace mm {
    
void heap::init() {
    heapBegin = pmm::alloc(0x100); /* allocate a 1mb heap */
    bitmap = (uint8_t*)(pmm::alloc(1) + HIGH_VMA);
    allocation = (allocation_t*)(pmm::alloc(0x20) + HIGH_VMA);

    bitmapSize = 0x100000 * 8;
    allocationSize = 0x100000 / sizeof(allocation_t);

    memset(bitmap, 0, 0x1000);
    memset(allocation, 0, 0x1000 * 0x20);
}

void *heap::kmalloc(uint64_t size) {
    if(size == 0) {
        cout + "[KMM]" << "stop trying to allocate zero bytes retard\n";
        return NULL;
    }

    int64_t cnt = 0, blockCount = ROUNDUP(size, BLOCKSIZE);
    void *base = (void*)(firstFreeSlot() * BLOCKSIZE); 

    for(int64_t i = firstFreeSlot(); i < bitmapSize; i++) {
        if(isset(bitmap, i)) {
            base = (void*)((uint64_t)base + (cnt + 1) * BLOCKSIZE);
            cnt = 0;
            continue;
        }

        if(++cnt == blockCount) {
            uint64_t slot = firstFreeAllocationSlot();
            
            allocation[slot].count = blockCount;
            allocation[slot].block = (uint64_t)base / BLOCKSIZE;

            for(int64_t j = 0; j < blockCount; j++) {
                set(bitmap, (uint64_t)base / BLOCKSIZE + j);
            }

            return (void*)((uint64_t)base + heapBegin + HIGH_VMA);
        }
    }

    cout + "[KMM]" << "Error: kheap is full\n";
    return NULL;
}

uint64_t heap::kfree(void *addr) {
    uint64_t bitmapBase = ((uint64_t)addr - HIGH_VMA - heapBegin) / 32, sizeOfAllocation = 0;

    int64_t i;
    for(i = 0; i < allocationSize; i++) {
        if(allocation[i].block == bitmapBase)
            break;
    }

    sizeOfAllocation = allocation[i].count;

    for(uint64_t j = bitmapBase; j < bitmapBase + sizeOfAllocation; j++) {
        clear(bitmap, j);
    }

    allocation[i].count = 0;
    allocation[i].block = 0;

    return sizeOfAllocation;
}

void *heap::krealloc(void *addr, uint64_t size) {
    uint64_t sizeOfAllocation = kfree(addr);
    void *newPt = kmalloc(sizeOfAllocation + size);
    memcpy64((uint64_t*)newPt, (uint64_t*)addr, sizeOfAllocation);
    return newPt; 
}

int64_t heap::firstFreeSlot() {
    for(int64_t i = 0; i < bitmapSize; i++) {
        if(!isset(bitmap, i))
            return i;
    }
    return -1;
}

int64_t heap::firstFreeAllocationSlot() {
    for(int64_t i = 0; i < allocationSize; i++) {
        if(allocation[i].count == 0) {
            return i;
        }
    }
    return -1;
}

}

void operator delete[](void *addr, uint64_t size) {
    kheap.kfree(addr);
}

void operator delete(void *addr, uint64_t size) {
    kheap.kfree(addr);
}
