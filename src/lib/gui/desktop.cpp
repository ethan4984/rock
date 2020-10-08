#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/sched/hpet.h>
#include <kernel/mm/kHeap.h>
#include <lib/stringUtils.h>
#include <lib/gui/desktop.h>
#include <lib/gui/text.h>

void timeAndDate() {
    textBox text(vesa::width - 20 * 8, 4, 20 * 8, 8, 0); 
    for(;;) {
        text.deleteAll();
        text.printf("lel"); 
        ksleep(1000);
    }
}

pannel::pannel(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour) : x(x),
    y(y), xCnt(xCnt), yCnt(yCnt), colour(colour) {
    pannelGrp = vesa::blkGrp(x, y, xCnt, yCnt, colour);
//    createTask(0x10, pmm::alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)timeAndDate, 2);
}
