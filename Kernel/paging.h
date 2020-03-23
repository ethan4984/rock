#pragma once

#include <stddef.h>

extern uint8_t *block_start;

namespace MM
{
    void set(uint64_t location);

    void clear(uint64_t location);

    uint8_t isset(uint64_t location);

    void *pagalloc(uint64_t size);

    void pagfree(void *location, size_t size = sizeof(uint64_t));

    void page_frame_init(uint64_t mem_range);

    uint64_t allocate_block();

    void free_block(uint64_t block_num);

    uint64_t first_free();

	void check_blocks(uint64_t range);

    void *new_address_space();

    void free_address_space(void *location);

    uint8_t *grab_start();
}

void page_setup();

void current_address_spaces();

void block_show();
