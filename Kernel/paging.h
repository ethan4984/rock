#pragma once

#include <stddef.h>

extern uint8_t *block_start;

namespace MM
{
    void set(uint64_t location);

    void clear(uint64_t location);

    uint8_t isset(uint64_t location);

    void *malloc(size_t size);

    void free(void *location, size_t size = sizeof(uint64_t));

    void page_frame_init(uint64_t mem_range);

    uint64_t allocate_block();

    void free_block(uint64_t block_num);

    uint64_t first_free();

	void check_blocks(uint64_t range);

    void *new_address_space();

    void free_address_space(void *location);

    uint8_t *grab_start();

    class virtual_address_space
    {
        public:
            void setup();
        protected:
            uint64_t page_dir[512] __attribute__((aligned(0x1000)));

            uint64_t page_tab[512] __attribute__((aligned(0x1000)));

            uint64_t page_dir_ptr_tab[4] __attribute__((aligned(0x20)));
    };
}

void page_setup();

void current_address_spaces();

void block_show();
