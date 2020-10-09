#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/sched/hpet.h>
#include <kernel/acpi/madt.h>
#include <kernel/sched/smp.h>
#include <kernel/int/apic.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <kernel/int/idt.h>
#include <kernel/int/gdt.h>
#include <kernel/int/tss.h>
#include <lib/asmUtils.h>
#include <lib/output.h>

static idtr_t idtr;

extern "C" symbol smpBegin;
extern "C" symbol smpEnd;

uint8_t cpuInfo_t::numberOfCores = 1;

uint8_t dynCnt = 1;

void prepTrampoline(uint64_t stack, uint64_t pml4, uint64_t entryPoint, uint64_t idt) {
    uint64_t *arguments = (uint64_t*)(0x500 + HIGH_VMA);
    arguments[0] = stack;
    arguments[1] = pml4;
    arguments[2] = entryPoint;
    arguments[3] = idt;
    arguments[4] = cpuInfo_t::numberOfCores++;
}

void bootstrapCoreMain() {
    apic::lapicWrite(LAPIC_SINT, apic::lapicRead(LAPIC_SINT) | 0x1ff); 

    cpuInfo[dynCnt].coreID = apic::lapicRead(LAPIC_ID_REG);
    cpuInfo[dynCnt].currentTask = -1;

    tss_t *tss = tss::create();
    gdt::initCore(cpuInfo_t::numberOfCores, (uint64_t)tss);

    apic::lapicTimerInit(100);

    asm volatile ("sti");

    cout + "[SMP]" << "Core " << dynCnt++ << " full initalized\n";

    for(;;);
}

void initSMP() {
    cpuInfo = new cpuInfo_t[madtInfo.madtEntry0Count * sizeof(cpuInfo_t)];
    memcpy8((uint8_t*)(0x1000 + HIGH_VMA), (uint8_t*)smpBegin, (uint64_t)smpEnd - (uint64_t)smpBegin);

    cpuInfo[0].currentTask = -1;
    asm volatile ("sidt %0" :: "m"(idtr));

    for(uint64_t i = 1; i < madtInfo.madtEntry0Count; i++) {
        uint64_t coreID = madtInfo.madtEntry0[i].apicID;
        if(madtInfo.madtEntry0[i].flags == 1) {
            prepTrampoline(pmm::alloc(4) + 0x4000 + HIGH_VMA, vmm::grabPML4(), (uint64_t)&bootstrapCoreMain, (uint64_t)&idtr);
            apic::sendIPI(coreID, 0x500);
            apic::sendIPI(coreID, 0x600 | (uint32_t)((uint64_t)0x1000 / 0x1000)); 
            ksleep(10);
        }
    }
}

void spinLock(char *ptr) {
    volatile uint64_t deadLockCnt = 0; 
    while (__atomic_test_and_set(ptr, __ATOMIC_ACQUIRE)) {
        if(++deadLockCnt == 100000000) {
            asm volatile ("int $0x15");
        }
    }
}

void spinRelease(char *ptr) {
    __atomic_clear(ptr, __ATOMIC_RELEASE);
}
