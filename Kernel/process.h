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

        void *pmalloc(size_t size);

        void pfree(void *location, size_t size = 0x80);

        uint32_t allocate_pblock();

        uint32_t first_freep();

        void free_pblock(size_t index);

        void show_blocks(int range);
    private:
        struct memory_table
        {
            uint32_t *available_blocks = { 0 };
        } pmem_map;
};

void free_process(process &ref);
