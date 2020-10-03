#include <kernel/mm/kHeap.h>
#include <kernel/int/tss.h>

void tssMain_t::init() {
    tss = new tss_t[32];
}

void tssMain_t::newTss(uint64_t rsp0) {
    static uint64_t cnt = 0;
    tss_t newTSS = { 0 };
    newTSS.rsp0 = rsp0; 
    tss[cnt++] = newTSS;
}
