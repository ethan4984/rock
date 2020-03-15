#pragma once

#include <stdint.h>
#include <stddef.h>

class process
{
    public:
        process(size_t range);

        int num_of_blocks = 0;

        uint32_t *process_begin;

        bool null_check();
};

void free_process(process &ref);
