#include <Kernel/mm/memoryParse.h>
#include <Slib/videoIO.h>

using namespace out;

E820map::E820map(stivaleInfo_t *bootInfo) : stivaleInfo(bootInfo) {
    cPrint("E820 parsing\n");
    mmap = (E820Entry_t*)bootInfo->memoryMapAddr;
    for(uint64_t i = 0; i < bootInfo->memoryMapEntries; i++) { /* runs through all of the mmap entries */
        totalMem += ((uint64_t)mmap[i].len + (uint64_t)mmap[i].addr) - (uint64_t)mmap[i].addr; /* the entry size */
    }
    cPrint("Total memory detected %d bytes", totalMem);
}

void E820map::getNextUseableMem(memoryRegion *memRegion) {
    static uint64_t lastRegion = 0;
    for(uint64_t i = lastRegion; i < stivaleInfo->memoryMapEntries; i++) {
        if(mmap[i].type == 1) { /* tpye = 1 : useable ram, type != 1 : reserved or bad */
            lastRegion = i + 1;
            memRegion->type = mmap[i].type;
            memRegion->mmapEntryNum = i;
            memRegion->base = mmap[i].addr;
            memRegion->limit = mmap[i].len + mmap[i].addr;
            return;
        }
    }
}

void E820map::getNthMmap(memoryRegion *memRegion, uint64_t num) {
    if(num > stivaleInfo->memoryMapEntries)
        num = stivaleInfo->memoryMapEntries;
    
    memRegion->type = mmap[num].type;
    memRegion->mmapEntryNum = num;
    memRegion->base = mmap[num].addr;
    memRegion->limit = mmap[num].len + mmap[num].addr;
}
