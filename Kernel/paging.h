#pragma once

#include <stddef.h>

extern uint8_t *block_start;

namespace MM
{
    void set(uint32_t location);

    void clear(uint32_t location);

    uint8_t isset(uint32_t location);

    void *malloc(size_t size);

    void free(void *location, size_t size = sizeof(uint32_t));

    void page_frame_init(uint32_t mem_range);

    uint32_t allocate_block();

    void free_block(uint32_t block_num);

    uint32_t first_free();

	void check_blocks(uint32_t range);

    void *new_address_space();

    void free_address_space(void *location);

    uint8_t *grab_start();

    class virtual_address_space
    {
        public:
            void new_virtual_map(uint32_t physical_addr, uint32_t virtual_addr);

            bool new_claim(uint32_t physical_addr, uint32_t virtual_addr);

            void *get_physical_address(void *virtual_addr);

            void identity_map_init();

            void *new_procces(); /* returns address a dynamic address space */

            uint32_t *physical_log;

            uint32_t *virtual_log;

            uint32_t size;
        protected:
            void enable_page();

            uint32_t *page_dir = 0;

            uint32_t page_loc = 0;

            uint32_t *last_page = 0;
    };
}

void page_setup();

void current_address_spaces();

void block_show();
