#include <Kernel/mm/physicalPageHandler.h>
#include <Kernel/mm/memHandler.h>
#include <Slib/memoryUtils.h>
#include <Slib/videoIO.h>

#include <stddef.h>

using namespace out;

uint8_t *blockBitmap; // each bit = 32 byte block

uint64_t numOfBlocks = 0x200000 / 32;  // 32 byte blocks
uint64_t numOfIndexs = 0x200000 / sizeof(allocationIndexs_t);
uint64_t numOfBlockAllocations = 0;

allocationIndexs_t *allocationIndexs;

void memInit(stivaleInfo_t *bootInfo)
{
    initPageHandler(bootInfo);

    blockBitmap = (uint8_t*)physicalPageAlloc(1);
    allocationIndexs = (allocationIndexs_t*)physicalPageAlloc(1);

    memset8(blockBitmap, 0, 0x200000);
}

uint64_t firstFreeBlock(uint64_t start = 0)
{
	for(uint64_t i = start; i < numOfBlocks; i++) {
		if(!isset(blockBitmap, i))
			return i;
	}
	return numOfBlocks + 1;
}

void *malloc(uint64_t size) 
{
    if(size == 0)
		size = 1;
	
	bool finished = 0;
    uint64_t realSize;

    if(size % 32 == size) 
        realSize = 1;
    else
        realSize = size / 32 + 1;

    uint64_t currentStartingAddress = firstFreeBlock();
	
	while(1) { /* this loop makes sure we have a continus stream of free blocks ready to allocate */
        if(currentStartingAddress == numOfBlocks) {
            blockBitmap = (uint8_t*)physicalPageRealloc(blockBitmap, 1);
            numOfBlocks += 0x200000 / 32;
            cPrint("here");
        }

		for(uint64_t i = currentStartingAddress; i < currentStartingAddress + realSize; i++) {
			if(isset(blockBitmap, i)) {
				currentStartingAddress = firstFreeBlock(i);
	            break;		
            }
			
			if(i == currentStartingAddress + realSize - 1) {
                finished = true;
                break;
            }
		}
		
		if(finished)
			break;
	}
	
    allocationIndexs_t entry;
    entry.address = (uint8_t*)(currentStartingAddress * 32 + (uint64_t)blockBitmap);
    entry.sizeOfAllocation = realSize;
    
    allocationIndexs[numOfBlockAllocations++] = entry;

    for(uint64_t i = currentStartingAddress; i < currentStartingAddress + realSize; i++) {
        set(blockBitmap, i); // Allocates it within the bitmap
    }

    return (void*)(currentStartingAddress * 32 + (uint64_t)blockBitmap);
}

void free(void *address)
{
    uint64_t locationIt = ((uint64_t)address - (uint64_t)blockBitmap) / 32;

    uint64_t sizeOfAllocation = 0; 

    for(uint64_t i = 0; i < numOfBlockAllocations; i++) {
        if(allocationIndexs[i].address == address) {
            sizeOfAllocation = allocationIndexs[i].sizeOfAllocation;
            break;
        }
    }

    for(uint64_t i = locationIt; i < locationIt + sizeOfAllocation; i++) {
        clear(blockBitmap, i);
    }
}

void *realloc(void *oldAddress, uint64_t size)
{
	asm volatile ("cli"); // disable interrupts becuase when we free we dont want the int handler to allocate anything
	
	uint64_t sizeOfAllocation = 0;

    for(uint64_t i = 0; i < numOfBlockAllocations; i++) {
        if(allocationIndexs[i].address == (uint8_t*)oldAddress) {
            sizeOfAllocation = allocationIndexs[i].sizeOfAllocation;
            break; 
        }
    }
    
    free(oldAddress);
	void *newAddress = malloc(sizeOfAllocation + size);
	memcpy(newAddress, oldAddress, sizeOfAllocation * 32);
	
	asm volatile ("sti");
	
	return newAddress;
}
