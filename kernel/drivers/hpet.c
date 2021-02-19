#include <drivers/hpet.h>
#include <mm/vmm.h>

static struct hpet_table *hpet_table;
static volatile struct hpet *hpet;

void ksleep(uint64_t ms) {
    uint64_t ticks = hpet->counter_value + (ms * 1000000000000) / ((hpet->capabilities >> 32) & 0xffffffff); 
    for(;hpet->counter_value < ticks;); 
}

void init_hpet() {
    hpet_table = find_SDT("HPET");
    *(volatile uint64_t*)(hpet_table->address + HIGH_VMA + 0x10) = 1; 
    hpet = (volatile struct hpet*)(hpet_table->address + HIGH_VMA);
}
