#include <Kernel/mm/physicalPageHandler.h> 
#include <Slib/videoIO.h>
#include <Slib/memoryUtils.h>

#include <stddef.h>

using namespace out;

typedef struct {
    void *addr;
    uint16_t sizeOfAllocation;
} pageAllocationInfo_t;

uint64_t totalUseableMemory = 0;
uint8_t *bitmap;
uint64_t totalNumOfPages = 0;
uint64_t numOfPageAllocations = 0;

pageAllocationInfo_t *pageAllocationInfo;

extern uint64_t kernelEnd;

void initPageHandler(stivaleInfo_t *bootInfo) 
{
    E820map e820(bootInfo);

    while(1) {     
        memoryRegion currentRegion;
        e820.getNextUseableMem(&currentRegion);

        cPrint("\n\nMemory Region found at phsyical address %x", currentRegion.base);
        cPrint("Goes until %x", currentRegion.limit);
        uint64_t entrySize = currentRegion.limit - currentRegion.base;
        cPrint("Total size: %d btytes", entrySize);
        totalUseableMemory += entrySize;

        if(currentRegion.mmapEntryNum == 8) 
            break;
    }

    totalNumOfPages = roundUp(e820.totalMem, 0x200000);
    
    cPrint("\n\nTotal memory: %d bytes", e820.totalMem);
    cPrint("Total useable memory: %d bytes", totalUseableMemory);
    cPrint("Total number of pages %d", totalNumOfPages);

    bitmap = (uint8_t*)(kernelEnd - 0x200000);
    pageAllocationInfo = (pageAllocationInfo_t*)(kernelEnd - 0x400000);

    memset8(bitmap, 0, totalNumOfPages);

    for(uint64_t i = 0; i < bootInfo->memoryMapEntries; i++) {
        memoryRegion currentRegion;
        e820.getNthMmap(&currentRegion, i);
        
        if(currentRegion.type != 1) { /* != 1 means reserved/bad in some form */
            uint64_t totalOccupiedPages = roundUp(currentRegion.limit - currentRegion.base, 0x200000); // mmap length / page size
            uint64_t bitmapStartingBit = roundUp(currentRegion.base, 0x200000); 
            allocateRegion(bitmapStartingBit, totalOccupiedPages);
        }
    }
    physicalPageAlloc(1); // allocate bios memory
}

uint64_t firstFreePage(uint64_t start = 0) 
{
    for(uint64_t i = start; i < totalNumOfPages; i++) {
        if(!isset(bitmap, i))
            return i;
    }
    return totalNumOfPages + 1;
}

void *physicalPageAlloc(uint64_t count)
{
    if(count == 0)
        count = 1;

    uint64_t currentAllocationStartingAddress = firstFreePage();

    bool finished = false;
    while(1) {
        if(currentAllocationStartingAddress > totalNumOfPages) {
            cPrint("Gamer you ran out of pages");
            return NULL;
        }

        /* makes sure that we done override any other existing pages */
        for(uint64_t i = currentAllocationStartingAddress; i < currentAllocationStartingAddress + count; i++) {
            if(isset(bitmap, i)) {
                currentAllocationStartingAddress = firstFreePage(i);
                continue;
            }

            if(i == currentAllocationStartingAddress + count - 1) {
                finished = true;
                break;
            }
        }
        
        if(finished)
            break;
    }

    pageAllocationInfo_t entry;
    entry.addr = (uint8_t*)((currentAllocationStartingAddress * 0x200000));
    entry.sizeOfAllocation = count; 

    pageAllocationInfo[numOfPageAllocations++] = entry;

    for(uint64_t i = currentAllocationStartingAddress; i < currentAllocationStartingAddress + count; i++) {
        set(bitmap, i); 
    }

    return (void*)((currentAllocationStartingAddress * 0x200000));
}

void physicalPageFree(void *address) 
{
    uint64_t locationIt = (uint64_t)address / 0x200000;

    uint64_t sizeOfAllocation = 0;

    for(uint64_t i = 0; i < numOfPageAllocations; i++) {
        if(pageAllocationInfo[i].addr == address) {
            sizeOfAllocation = pageAllocationInfo[i].sizeOfAllocation;
            break; 
        }
    }

    for(uint64_t i = locationIt; i < locationIt + sizeOfAllocation; i++) {
        clear(bitmap, i);
    }
}

void *physicalPageRealloc(void *oldAddress, uint64_t size)
{
    asm volatile ("cli"); // disable interrupts becuase when we free we dont want the int handler to allocate anything

    uint64_t sizeOfAllocation = 0;

    for(uint64_t i = 0; i < numOfPageAllocations; i++) {
        if(pageAllocationInfo[i].addr == (uint8_t*)oldAddress) {
            sizeOfAllocation = pageAllocationInfo[i].sizeOfAllocation;
            break; 
        }
    }

    physicalPageFree(oldAddress);
    void *newAddress = physicalPageAlloc(sizeOfAllocation + size);
    memcpy(newAddress, oldAddress, sizeOfAllocation * 0x200000);

    asm volatile ("sti");

    return newAddress;
}

void allocateRegion(uint64_t start, uint64_t end)
{
    for(uint64_t i = start; i < end; i++) {
        set(bitmap, i);
    }
}

uint64_t roundUp(uint64_t divided, uint64_t divisor) 
{
    if(divided % divisor == 0) {
        return divided / divisor;
    } else {
        return (divided / divisor) + 1;
    }
}
