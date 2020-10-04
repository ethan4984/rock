#include <kernel/mm/physicalPageManager.h> 
#include <kernel/mm/kHeap.h>
#include <kernel/int/tss.h>

namespace tss {

tss_t *tssEntries;

void init() {
    tssEntries = new tss_t[32];
}

tss_t *create() {
    static uint64_t cnt = 0;
    tss_t tss = { 0 };
    tss.rsp0 = pmm::alloc(2);
    tss.rsp1 = pmm::alloc(2);
    tss.rsp2 = pmm::alloc(2);
    tssEntries[cnt] = tss;
    return &tssEntries[cnt++];
}

}
