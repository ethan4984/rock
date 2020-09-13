#pragma once

#include <lib/memoryUtils.h>

#include <stdint.h>

namespace kernel {

class widget {
public:
    widget(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour, function<void, uint32_t, uint32_t> clickHandler);

    widget() = default;
    
    void inRange(uint32_t x, uint32_t y);
private:    
    uint32_t x, y, xCnt, yCnt, colour;

    function<void, uint32_t, uint32_t> clickHandler;
};

void createWidget(widget newWidget);

inline widget *widgets;
inline uint32_t widgetCnt = 0;

}
