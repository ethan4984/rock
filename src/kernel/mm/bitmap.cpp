#include <kernel/sched/smp.h>
#include <kernel/mm/bitmap.h>
#include <lib/memoryUtils.h>
#include <lib/output.h>

namespace mm {

void bitmapHeap::init(uint64_t pageCnt) {
    start = pmm::alloc(pageCnt);

    bitmap = (uint8_t*)(pmm::alloc(ROUNDUP(0x1000 * pageCnt, BLOCK_SIZE) / 8) + HIGH_VMA);
    bitmapSize = ROUNDUP(0x1000 * pageCnt, BLOCK_SIZE) / 8;
	
    allocations = (allocation_t*)(pmm::alloc(0x20) + HIGH_VMA);
    allocationSize = (0x20 * 0x1000);
	
    memset(bitmap, 0, bitmapSize);
    memset(allocations, 0, allocationSize);
}

void *bitmapHeap::alloc(uint64_t size) {
    spinLock(&lock);

    if(size == 0) {
        cout + "[KMM]" << "stop trying to allocate zero bytes retard\n";
        return NULL;
    }

    uint32_t cnt = 0, blockCount = ROUNDUP(size, BLOCK_SIZE);
    void *base = (void*)(firstFreeSlot() * BLOCK_SIZE); 

    for(uint64_t i = firstFreeSlot(); i < bitmapSize; i++) {
        if(isset(bitmap, i)) {
            base = (void*)((uint64_t)base + (cnt + 1) * BLOCK_SIZE);
            cnt = 0;
            continue;
        }

        if(++cnt == blockCount) {
            uint64_t slot = firstFreeAllocationSlot();

            allocations[slot] = { blockCount, (uint32_t)((uint64_t)base / BLOCK_SIZE) };

            for(int64_t j = 0; j < blockCount; j++) {
                set(bitmap, (uint64_t)base / BLOCK_SIZE + j);
            }

            spinRelease(&lock);
            return (void*)((uint64_t)base + start + HIGH_VMA);
        }
    }

    kprintDS("[KDEBUG]", "%d", size); 

    cout + "[KMM]" << "Error: heap is full\n";
    spinRelease(&lock);
    return NULL; 
};

uint64_t bitmapHeap::free(void *addr) {
    spinLock(&lock);
    uint64_t bitmapBase = ((uint64_t)addr - HIGH_VMA - start) / BLOCK_SIZE, sizeOfAllocation = 0;

    size_t i;
    for(i = 0; i < allocationSize; i++) {
        if(allocations[i].blkIdx == bitmapBase)
            break;
    }

    sizeOfAllocation = allocations[i].blkCnt;

    for(uint64_t j = bitmapBase; j < bitmapBase + sizeOfAllocation; j++) {
        clear(bitmap, j);
    }

    allocations[i] = { };

    spinRelease(&lock);
    return sizeOfAllocation;
}

void *bitmapHeap::realloc(void *addr, uint64_t size) {
    spinLock(&lock);

    uint64_t sizeOfAllocation = free(addr);
    void *newPt = alloc(sizeOfAllocation + size);
    memcpy64((uint64_t*)newPt, (uint64_t*)addr, sizeOfAllocation);

    spinRelease(&lock); 
    return newPt; 
}

int64_t bitmapHeap::firstFreeSlot() {
    for(size_t i = 0; i < bitmapSize; i++) {
        if(!isset(bitmap, i))
            return i;
    }
    return -1;
}

int64_t bitmapHeap::firstFreeAllocationSlot() {
	for(size_t i = 0; i < allocationSize; i++) {
        if(allocations[i].blkCnt == 0) {
            return i;
        }
    }
    return -1;
}
	
}
