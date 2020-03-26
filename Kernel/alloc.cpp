#include <shitio.h>
#include <alloc.h>
#include <paging.h>
#include <interrupt.h>

using namespace standardout;
using namespace MM;

uint64_t *bitmap_b;
uint64_t *block_start_b;
const uint64_t total_blocks_b = 20000;
uint64_t total_used_blocks = 0;

using namespace MM;

void blocks_init()
{
    block_start_b = (uint64_t*)pagalloc(1);

    static uint64_t holder = 0;
    uint64_t i;
    for(i = 0; i < (holder + 20000); i++)
        bitmap_b[i] = 0;

    holder = i;
}

uint64_t first_bfree()
{
    for(uint64_t i = 0; i < total_blocks_b; i++) {
        if(bitmap_b[i] == 0)
            return i;
    }
    t_print("Bruh: we are running out a blocks, make some more bitch\n");
    return total_blocks_b + 1;
}

uint64_t allocate_bblock()
{
    uint64_t new_block = first_bfree();
    t_print("\tAllocated block: %d", new_block);
    bitmap_b[new_block] = 1;
    return new_block;
}

void free_bblock(uint64_t block_num)
{
    t_print("freeing %d", block_num);
    bitmap_b[block_num] = 0;
}

void *malloc(uint64_t size)
{
    t_print("\nMALLOC Allocation in process\n");

    if(!size) {
        t_print("MALLOC: wtf are you doing trying to allocate a blocks of zero size");
        return 0;
    }

    uint64_t reqiured_blocks = 1;

    while(reqiured_blocks < size)
        reqiured_blocks++;

    uint64_t freed = first_bfree();

    t_print("\tStatus: Multi-block\n\tBlocks reqiured: %d\n", reqiured_blocks);

    uint64_t i;
    for(i = 0; i < reqiured_blocks; i++) {
        if(allocate_bblock() == total_blocks_b + 1) {
            t_print("BRUH: we ran out of blocks bruh");
            panic("We ran out of blocks");
        }
    }
    t_print("\nMALLOC allocation finished\n");

    if((uint64_t*)block_start_b + (freed*i) > (uint64_t*)block_start_b + 0x20000)
        blocks_init();

    t_print("bro %x", (uint64_t)block_start_b + (freed*i));

    return block_start_b + (freed*i);
}

void *realloc(void *location)
{
    if(location == NULL) {
        t_print("REALLOC: wtf bruh, your trying to realloc NULL memory");
        return NULL;
    }

    uint64_t memory = (uint64_t)location;

    if(first_bfree() > (memory - (uint64_t)block_start_b)) {
        t_print("REALLOC: you dont need to reallocate");
        return (uint64_t*)memory;
    }
}

void free(void *location)
{
    if(location == 0) {
        t_print("MALLOC: wtf are you doing trying to free NULL memory");
        return;
    }

    uint64_t yest = (uint64_t)location;

    uint64_t reqiured_blocks = 0;

    free_bblock((yest - (uint64_t)block_start_b));
    for(uint64_t i = 0; i < reqiured_blocks; i++)
        t_print("this block was just freed %x", (yest - (uint64_t)block_start_b));
}

void check_blocks_b()
{
    for(uint64_t i = 0; i < 10; i++) {
        if(bitmap_b[i] == 0)
            t_print("This shit is free");
        else
            t_print("Taken");
    }
}
