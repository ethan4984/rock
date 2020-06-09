#include <Kernel/drivers/keyboard.h>
#include <Kernel/drivers/vesa.h>
#include <Kernel/mm/memHandler.h>
#include <Kernel/mm/memoryParse.h>
#include <Kernel/pci.h>
#include <Kernel/sched/scheduler.h>
#include <Kernel/stivale.h>
#include <Slib/stringUtils.h>
#include <Slib/memoryUtils.h>
#include <Slib/videoIO.h>
#include <prompt.h>

using namespace out;

void infoMMAP() 
{
    stivaleInfo_t *stivaleInfo = getstivale();
    E820Entry_t *mmap = (E820Entry_t*)stivaleInfo->memoryMapAddr;
    uint64_t totalMemory = 0, totalUseableMemory = 0;
    for(uint64_t i = 0; i < stivaleInfo->memoryMapEntries; i++)  {
        if(mmap[i].type == 1) {
            totalUseableMemory +=  ((uint64_t)mmap[i].len + (uint64_t)mmap[i].addr) - (uint64_t)mmap[i].addr; 
            totalMemory +=  ((uint64_t)mmap[i].len + (uint64_t)mmap[i].addr) - (uint64_t)mmap[i].addr; 
        }
        else 
            totalMemory +=  ((uint64_t)mmap[i].len + (uint64_t)mmap[i].addr) - (uint64_t)mmap[i].addr; 
        kPrint("Entry %d [%x] -> [%x] -- %s\n\n", i, mmap[i].addr, mmap[i].len + mmap[i].addr, mmapEntryType(mmap[i].type)); 
    }
    kPrint("Total Memory %d ~ Total Useable memory %d\n", totalMemory, totalUseableMemory);
}

void test() 
{
    cPrint("test");
}

const char *commandList[] = { "info mmap", "info pci", "test" };
int commandSize = 0;

typedef void (*commandFunctions)();

commandFunctions commandHandlers[] = { infoMMAP, showDevices, test };

void promptMain()
{
    kPrint("> ");

    grabKeys();

    while(1) {
        if(isDone())
            break;

        if(oneChar()) {
            putchar(grabKey());
            commandSize++;    
            falseFound();
        }
    }
    
    putchar('\n');
    
    char *command = getKeys();

    bool found = false;

    for(int i = 0; i < 3; i++) {
        if(!strcmp(command, commandList[i])) {
            commandHandlers[i]();
            found = true;
            break;
        }
    }

    if(!found && commandSize)
        kPrint("Command \"%s\" not found\n", command);

    free(command);

    memset(command, 0, commandSize);
    commandSize = 0;

    promptMain();
}
