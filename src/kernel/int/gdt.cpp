#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <kernel/int/gdt.h>

namespace gdt {

void init() {
    gdtCores = new gdtCore_t[32];
    memset(gdtCores, 0, sizeof(gdtCore_t[32]));
}

void initCore(uint64_t core, uint64_t tssAddr) {
    /* code 64 */

    gdtCores[core].gdtEntries[1].limit = 0;
    gdtCores[core].gdtEntries[1].baseLow = 0;
    gdtCores[core].gdtEntries[1].baseMid = 0;
    gdtCores[core].gdtEntries[1].access = 0b10011010; 
    gdtCores[core].gdtEntries[1].granularity = 0b00100000;
    gdtCores[core].gdtEntries[1].baseHigh = 0;

    /* data 64 */

    gdtCores[core].gdtEntries[2].limit = 0;
    gdtCores[core].gdtEntries[2].baseLow = 0;
    gdtCores[core].gdtEntries[2].baseMid = 0;
    gdtCores[core].gdtEntries[2].access = 0b10010110; 
    gdtCores[core].gdtEntries[2].granularity = 0;
    gdtCores[core].gdtEntries[2].baseHigh = 0;

    /* user code 64 */

    gdtCores[core].gdtEntries[3].limit = 0;
    gdtCores[core].gdtEntries[3].baseLow = 0;
    gdtCores[core].gdtEntries[3].baseMid = 0;
    gdtCores[core].gdtEntries[3].access = 0b11111101; 
    gdtCores[core].gdtEntries[3].granularity = 0b10101111;
    gdtCores[core].gdtEntries[3].baseHigh = 0;

    /* user data 64 */

    gdtCores[core].gdtEntries[4].limit = 0;
    gdtCores[core].gdtEntries[4].baseLow = 0;
    gdtCores[core].gdtEntries[4].baseMid = 0;
    gdtCores[core].gdtEntries[4].access = 0b11110011; 
    gdtCores[core].gdtEntries[4].granularity = 0b11001111;
    gdtCores[core].gdtEntries[4].baseHigh = 0;

    /* tss */

    gdtCores[core].tss.length = 104;
    gdtCores[core].tss.baseLow = (uint16_t)(uint64_t)tssAddr;
    gdtCores[core].tss.baseMid = (uint8_t)((uint64_t)tssAddr >> 16);
    gdtCores[core].tss.flags1 = 0b10001001; 
    gdtCores[core].tss.flags2 = 0;
    gdtCores[core].tss.baseHigh = (uint8_t)((uint64_t)tssAddr >> 24);
    gdtCores[core].tss.baseHigh32 = (uint32_t)((uint64_t)tssAddr >> 32);

    gdtCores[core].gdtr.limit = sizeof(gdtCore_t) - sizeof(gdtr_t) - 1;
    gdtCores[core].gdtr.offset = (uint64_t)&gdtCores[core];

    lgdt((uint64_t)&gdtCores[core].gdtr, tssAddr);
}

}
