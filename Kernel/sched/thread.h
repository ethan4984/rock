#pragma once

#include <stdint.h>

class thread
{
    public:
        thread(uint8_t status_t, uint64_t rbp_t, uint64_t rsp_t, void *main);

        uint8_t status;
        uint64_t rbp;
        uint64_t rsp;
        void *task;
};
