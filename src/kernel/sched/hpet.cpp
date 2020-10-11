#include <kernel/mm/virtualPageManager.h>
#include <kernel/sched/hpet.h>

hpetTable_t *hpetTable;
volatile hpet_t *hpet;

void ksleep(uint64_t ms) {
    uint64_t ticks = hpet->counterValue + (ms * 1000000000000) / ((hpet->capabilities >> 32) & 0xffffffff); 
    while(hpet->counterValue < ticks);
}

void initHPET() {
    hpetTable = acpi::findSDT<hpetTable_t>("HPET");
    *(volatile uint64_t*)(hpetTable->address + HIGH_VMA + 0x10) = 1;
    hpet = (hpet_t*)(hpetTable->address + HIGH_VMA);
}
