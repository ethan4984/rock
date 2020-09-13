#pragma once

#include <lib/memoryUtils.h>
#include <lib/asmUtils.h>
#include <lib/vesa.h>

namespace kernel {

class pannel {
public:
    pannel(uint32_t x, uint32_t y, uint32_t xCnt, uint32_t yCnt, uint32_t colour);
private:
    VesaBlkGrp pannelGrp;

    uint32_t x, y, xCnt, yCnt, colour;
};

}
