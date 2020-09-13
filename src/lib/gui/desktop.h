#pragma once

#include <lib/memoryUtils.h>
#include <lib/asmUtils.h>
#include <lib/vesa.h>

namespace kernel {

class widget {
public:
    widget(function<int, uint32_t, uint32_t> handler);

    widget() = default;
private:
    function<int, uint32_t, uint32_t> handler;
};

class pannel {
public:
    pannel(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour);

    void addWidget(widget newWidget);
private:
    widget *widgets;

    VesaBlkGrp pannelGrp;

    uint32_t totalWidgets, x, y, xCnt, yCnt, colour;
};

}
