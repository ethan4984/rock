#include <kernel/sched/hpet.h>
#include <kernel/mm/kHeap.h>
#include <lib/gui/desktop.h>

namespace kernel {

widget::widget(function<int, uint32_t, uint32_t> handler) : handler(handler) {
    
}

pannel::pannel(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour) : x(x),
    y(y), xCnt(xCnt), yCnt(yCnt), colour(colour) {
    widgets = new widget[10];
    pannelGrp = VesaBlkGrp(x, y, xCnt, yCnt, colour);
}

void pannel::addWidget(widget newWidget) {
    if(++totalWidgets % 10 == 0) {
        widgets = (widget*)kheap.krealloc(widgets, 10);
    }
    widgets[totalWidgets - 1] = newWidget;
}

}
