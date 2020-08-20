#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/sched/hpet.h>
#include <kernel/acpi/madt.h>
#include <kernel/sched/smp.h>
#include <kernel/int/apic.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <kernel/int/idt.h>
#include <lib/asmUtils.h>

namespace kernel {

static idtr_t idtr;

extern "C" symbol smpBegin;
extern "C" symbol smpEnd;

uint8_t cpuInfo_t::numberOfCores;

void prepTrampoline(uint64_t stack, uint64_t pml4, uint64_t entryPoint, uint64_t idt) {
    uint64_t *arguments = (uint64_t*)(0x500 + HIGH_VMA);
    arguments[0] = stack;
    arguments[1] = pml4;
    arguments[2] = entryPoint;
    arguments[3] = idt;
    arguments[4] = cpuInfo_t::numberOfCores++;
}

void bootstrapCoreMain() {
    for(;;);
}

void initSMP() {
    cpuInfo = new cpuInfo_t[madtInfo.madtEntry0Count * sizeof(cpuInfo_t)];
    memcpy8((uint8_t*)0x1000, (uint8_t*)smpBegin, smpEnd - smpBegin);

    cpuInfo[0].currentTask = -1;
    asm volatile ("sidt %0" :: "m"(idtr));

    for(uint64_t i = 0; i < madtInfo.madtEntry0Count; i++) {
        uint64_t coreID = madtInfo.madtEntry0[i].apicID;
        if(madtInfo.madtEntry0[i].flags == 1) {
            prepTrampoline(physicalPageManager.alloc(4) + 0x4000 + HIGH_VMA, virtualPageManager.grabPML4(), (uint64_t)&bootstrapCoreMain, (uint64_t)&idtr);        
            apic.sendIPI(coreID, 0x500); // init ipi
            apic.sendIPI(coreID, 0x600 | (uint32_t)(0x1000ull / PAGESIZE)); 
            ksleep(10);
        } 
    }
}

}
