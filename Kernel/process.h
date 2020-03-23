#pragma once

#include <stdint.h>
#include <stddef.h>

class process
{
    public:
        process(uint64_t range, void (*entry)());

        process();

        ~process();

        uint64_t num_of_blocks = 0;

        uint64_t *process_begin;

        bool null_check();

        void *pmalloc(size_t size);

        void pfree(void *location, size_t size = 0x80);

        uint64_t allocate_pblock();

        uint64_t first_freep();

        void free_pblock(uint64_t index);

        void show_blocks(uint64_t range);

        void (*entry_point)();

        void save_regs();

        void restore();

        void operator =(const process &obj)
        {
			num_of_blocks = obj.num_of_blocks;
			process_begin = obj.process_begin;
		}
    private:
        struct memory_table
        {
            uint64_t *available_blocks = { 0 };
        } pmem_map;

        uint64_t rax;
        uint64_t rbx;
        uint64_t rcx;
        uint64_t rdx;

        uint64_t rdi;
        uint64_t rsi;
        uint64_t rbp;
        uint64_t rsp;

        uint64_t r8;
        uint64_t r9;
        uint64_t r10;
        uint64_t r11;
        uint64_t r12;
        uint64_t r13;
        uint64_t r14;
        uint64_t r15;

        uint64_t new_rsp;
        uint64_t up_time = 0;
};

extern void tss_flush();

void free_process(process &ref);

void task_switch(process &task);
