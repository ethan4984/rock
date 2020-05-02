#include <shitio.h>
#include <alloc.h>
#include <paging.h>
#include <interrupt.h>
#include <memory.h>

using namespace standardout;
using namespace MM;

uint8_t *bitmap_b;
uint8_t *bitmap_i;
uint64_t *block_start_b;
uint64_t bitmap_size;

using namespace MM;

void set(uint64_t location)
{
    bitmap_b[location / 8] = bitmap_b[location / 8] | (1 << (location % 8));
}

void clear(uint64_t location)
{
    bitmap_b[location / 8] = bitmap_b[location / 8] & (~(1 << (location % 8)));
}

bool isset(uint64_t location)
{
    return (bitmap_b[location / 8] >> (location % 8)) & 0x1;
}

void reserve(uint64_t location)
{
    bitmap_i[location / 8] = bitmap_i[location / 8] | (1 << (location % 8));
}

void sell(uint64_t location)
{
    bitmap_i[location / 64] = bitmap_i[location / 64] & (~(1 << (location % 64)));
}

bool is_reserved(uint64_t location)
{
    return (bitmap_i[location / 8] >> (location % 8)) & 0x1;
}

void blocks_init()
{
    block_start_b = (uint64_t*)pagalloc(1);

    bitmap_b = (uint8_t*)block_start_b - 8;
    bitmap_i = (uint8_t*)block_start_b - 16;

    bitmap_size = 0x20000;

    memset(bitmap_b, 0, bitmap_size / 8);
}

uint64_t first_bfree(uint64_t start)
{
    for(uint64_t i = start; i < bitmap_size; i++) {
        if(!isset(i))
            return i;
    }
    t_print("out of blocks, allocating more\n");
    return bitmap_size + 1;
}

uint64_t allocate_bblock(uint64_t start)
{
    uint64_t new_block = first_bfree(start);
    //t_print("\tAllocated block: %d\n", new_block);
    set(new_block);
    return new_block;
}

void *malloc(uint64_t size)
{
//    t_print("\nMALLOC Allocation in process\n");

    if(!size) {
        t_print("MALLOC: wtf why are you trying to allocate a block with no size");
        return 0;
    }

    uint64_t freed = first_bfree(0), offset = 0;

    //t_print("\tStatus: Multi-block\n\tBlocks required: %d\n", size);
    int error_check = 0;
    while(1) {
        uint64_t check_size = 0;
        for(int i = freed; i < freed + size; i++) {
            if(!isset(i))
                check_size++;
        }

        if(check_size == size) {
            offset = freed;
            break;
        }
        freed = first_bfree((freed + size) - 1);
        if(++error_check == bitmap_size) {
            k_print("You ran out of blocks retard");
            panic("Bruh moment");
        }
    }

    reserve(freed);

    uint64_t i;
    for(i = 0; i < size; i++) {
        if(allocate_bblock(offset) == bitmap_size + 1) {
            t_print("BRUH: we ran out of blocks bruh");
            panic("We ran out of blocks");
        }
    }

    reserve(first_bfree(offset));

    uint64_t return_address = (uint64_t)block_start_b + freed;

    //t_print("MALLOC allocation finished at %x", return_address);

    if((uint64_t*)block_start_b + freed > (uint64_t*)block_start_b + 0x20000) /* test me */
        blocks_init();

    return (uint64_t*)return_address;
}

void free(void *location)
{
    if(location == 0) {
        t_print("MALLOC: wtf why are you trying to free NULL memory");
        return;
    }

    uint64_t current_block = ((uint64_t)location - (uint64_t)block_start_b);

    t_print("Freeing in process: \n");
    t_print("\tAddr: %x\n", location);
    t_print("\tBlock Numbers: %d\n", current_block);

    while(1) {
        clear(current_block);
        if(is_reserved(++current_block))
            break;
    }

    sell(current_block);

    t_print("Freeing finished");
}

void check_blocks_b()
{
    for(uint64_t i = 0; i < 10; i++) {
        if(!isset(i))
            t_print("This shit is free");
        else
            t_print("Taken");
    }
}
