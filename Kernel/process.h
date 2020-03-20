#pragma once

#include <stdint.h>
#include <stddef.h>

class process
{
    public:
        process(size_t range);

        uint64_t num_of_blocks = 0;

        uint64_t *process_begin;

        bool null_check();

        void *pmalloc(size_t size);

        void pfree(void *location, size_t size = 0x80);

        uint32_t allocate_pblock();

        uint32_t first_freep();

        void free_pblock(uint64_t index);

        void show_blocks(uint64_t range);
    private:
        struct memory_table
        {
            uint64_t *available_blocks = { 0 };
        } pmem_map;
};

extern void tss_flush();

struct stack_switcher {
    uint64_t esp0;
    uint64_t esp1;
    uint64_t eip0;
    uint64_t eip1;

    stack_switcher *next;
} __attribute__ ((packed));

void new_stack(uint64_t address);

void free_process(process &ref);
