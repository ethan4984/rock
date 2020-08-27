#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <lib/memoryUtils.h>
#include <lib/asmUtils.h>
#include <lib/output.h>

namespace kernel {

void physicalPageManager_t::init(stivaleInfo_t *stivaleInfo) {
    stivaleMMAPentry_t *stivaleMMAPentry = (stivaleMMAPentry_t*)stivaleInfo->memoryMapAddr;
    for(uint64_t i = 0; i < stivaleInfo->memoryMapEntries; i++) 
        cout + "[KMM]" << "[" << stivaleMMAPentry[i].addr << " -> " << stivaleMMAPentry[i].addr + stivaleMMAPentry[i].len << "] : length " << stivaleMMAPentry[i].len << " type " << stivaleMMAPentry[i].type << "\n";

    for(uint64_t i = 0; i < stivaleInfo->memoryMapEntries; i++) { /* find a location for the bitmap */
        totalDetectedMemory += stivaleMMAPentry[i].len;
        if((stivaleMMAPentry[i].type == 1) && stivaleMMAPentry[i].len >= 96 * 0x1000) {
            bitmap = (uint8_t*)(stivaleMMAPentry[i].addr + HIGH_VMA);
            allocateRegion(stivaleMMAPentry[i].addr, 96 * 0x1000); /* mark bitmap memory as alloacted */
            break;
        }
    }

    memset32((uint32_t*)bitmap, 0, (96 * 0x1000) / 4);

    for(uint64_t i = 0; i < stivaleInfo->memoryMapEntries; i++) {
        if(stivaleMMAPentry[i].type == 1) { /* 1 is the only useable type */ 
            freeRegion(stivaleMMAPentry[i].addr, stivaleMMAPentry[i].len);
        }
    }

    allocateRegion(0, 0x100000); /* mark everything below 1mb cuz its hell down there */

    cout + "[KMM]" << "Physical Memory Manager initalized\n";
    cout + "[KMM]" << "Total Detected Memory: " << totalDetectedMemory << "\n";
}

void physicalPageManager_t::allocateRegion(uint64_t start, uint64_t limit) {
    for(uint64_t i = start / PAGESIZE; i < (start / PAGESIZE) + ROUNDUP(limit, PAGESIZE); i++) {
        set(bitmap, i);
    }
}

void physicalPageManager_t::freeRegion(uint64_t start, uint64_t limit) {
    for(uint64_t i = start / PAGESIZE; i < (start / PAGESIZE) + ROUNDUP(limit, PAGESIZE); i++) {
        clear(bitmap, i);
    }
}

uint64_t physicalPageManager_t::alloc(uint64_t cnt) {
    uint64_t base = firstFreePage() * PAGESIZE, count = 0;
    for(uint64_t i = firstFreePage(); i < totalDetectedMemory / PAGESIZE; i++) {
        if(isset(bitmap, i)) {
            base += (cnt + 1) * PAGESIZE;
            cnt = 0;
            continue;
        }

        if(++count == cnt) {
            for(uint64_t j = 0; j < count; j++)
                set(bitmap, base / PAGESIZE + j);
            return base;
        }
    }
    
    cout + "[KMM]" << "Error: could not alloacte " << cnt << " consecutive pages\n";

    return 0;
}

void physicalPageManager_t::free(uint64_t base, uint64_t count) {
    for(uint64_t i = ROUNDUP((uint64_t)base, PAGESIZE); i < ROUNDUP((uint64_t)base, PAGESIZE) + count; i++) {
        clear(bitmap, i);
    }
}

int64_t physicalPageManager_t::firstFreePage() {
    for(uint64_t i = 0; i < totalDetectedMemory / PAGESIZE; i++) {
        if(!isset(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

}
