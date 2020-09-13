#include <kernel/mm/kHeap.h>
#include <lib/gui/widget.h>
#include <lib/output.h>
#include <lib/vesa.h>

namespace kernel {

widget::widget(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour, function<void, uint32_t, uint32_t> clickHandler) : x(x),
    y(y), xCnt(xCnt), yCnt(yCnt), colour(colour), clickHandler(clickHandler) {
    VesaBlkGrp(x, y, xCnt, yCnt, colour); 
}

void widget::inRange(uint32_t x, uint32_t y) {
    if(x >= this->x && (this->x + (this->xCnt * 8) > x)) {
        if(y >= this->y && this->y + (this->yCnt * 8) > y) {
            clickHandler(this->x, this->y); 
        }
    } 
}

void createWidget(widget newWidget) {
    if(widgetCnt == 0) {
        widgets = new widget[10];
    }

    if(++widgetCnt % 10 == 0) {
        widgets = (widget*)kheap.krealloc(widgets, 10);
    }

    widgets[widgetCnt - 1] = newWidget;
}

}
