#include <drivers/hpet.h>
#include <mm/vmm.h>

static hpet_table_t *hpet_table;
static volatile hpet_t *hpet;

void ksleep(uint64_t ms) {
    uint64_t ticks = hpet->counter_value + (ms * 1000000000000) / ((hpet->capabilities >> 32) & 0xffffffff); 
    for(;hpet->counter_value < ticks;); 
}

void init_hpet() {
    hpet_table = find_SDT("HPET");
    *(volatile uint64_t*)(hpet_table->address + HIGH_VMA + 0x10) = 1; 
    hpet = (hpet_t*)(hpet_table->address + HIGH_VMA);
}
