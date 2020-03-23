#include <stdint.h>

#include <paging.h>
#include <shitio.h>
#include <memory.h>
#include <interrupt.h>

#define bitmap_begin 0x80173000

using namespace standardout;

extern uint64_t kernel_end;

uint64_t bitmap_start = 0;

uint8_t *bitmap = (uint8_t*)(&kernel_end);
uint8_t *block_start;
uint8_t *block_reference;
uint64_t total_blocks;
uint64_t size;

namespace MM
{
    void set(uint64_t location)
    {
        bitmap[location / 8] = bitmap[location / 8] | (1 << (location % 8));
    }

    void clear(uint64_t location)
    {
        bitmap[location / 8] = bitmap[location / 8] & (~(1 << (location % 8)));
    }

    uint8_t isset(uint64_t location)
    {
        return (bitmap[location / 8] >> (location % 8)) & 0x1;
    }

    void page_frame_init(uint64_t mem_range)
    {
        total_blocks = mem_range / 0x20000; //4Kb blocks

        size = total_blocks / 8;
        if(size * 16 < total_blocks)
            size++;

        memset(bitmap, 0, size);
        block_start = (uint8_t*)((uint64_t)(bitmap + size) + 0x20000); //0x80174000
        bitmap_start = (uint64_t)block_start;
    }


    uint64_t allocate_block()
    {
        uint64_t new_block = first_free();
		t_print("\tAllocated block: %d", new_block);
        set(new_block);
        return new_block;
    }

    void free_block(uint64_t block_num)
    {
        t_print("freeing %d", block_num);
        clear(block_num);
    }

    uint64_t first_free()
    {
        for(uint64_t i = 0; i < total_blocks; i++) {
            if(!isset(i))
                return i;
        }
        t_print("Bruh: we are running out a blocks, make some more bitch\n");
        return total_blocks + 1;
    }

    void *pagalloc(uint64_t size)
    {
        t_print("\nBlock Allocation in process\n");

        if(!size) {
            t_print("BRUH: wtf are you doing trying to allocate a blocks of zero size");
            return 0;
        }

        uint64_t reqiured_blocks = 1;

        bool is_one = true;

        while(reqiured_blocks*0x20000 < size) {
            reqiured_blocks++;
            is_one = false;
        }

		uint64_t freed = first_free();

        if(is_one) {
            if(first_free() == total_blocks + 1) {
                t_print("BRUH: we ran out of blocks bruh");
                panic("We ran out of blocks");
            }

            t_print("\tStatus: Single Block\n");

			allocate_block();

            t_print("\nBlock allocation finished : %x\n", block_start + freed*0x20000);

            return block_start + freed*0x20000;
        }

        t_print("\tStatus: Multi-block\n\tBlocks reqiured: %d\n", reqiured_blocks);

        uint64_t i;
        for(i = 0; i < reqiured_blocks; i++) {
            if(allocate_block() == total_blocks + 1) {
                t_print("BRUH: we ran out of blocks bruh");
                panic("We ran out of blocks");
            }
        }
        t_print("\nBlock allocation finished\n");
        return block_start + (freed*i)*0x20000; //fix me
    }

    void pagfree(void *location, size_t size)
    {
        if(location == NULL) {
            t_print("BRUH: wtf are you doing trying to free NULL memory");
            return;
        }

        uint64_t yest = (uint64_t)location;

        uint64_t reqiured_blocks = 0;
        while(++reqiured_blocks*0x20000 < size);

        if(reqiured_blocks == 1) {
            free_block((yest - bitmap_start) / 0x20000);
            t_print("trying to free %x", (yest - bitmap_start) / 0x20000);
            t_print("this block was just freed %x", (yest - bitmap_start) / 0x20000);
            return;
        }

        free_block((yest - bitmap_start) / 0x20000);
        for(uint64_t i = 0; i < reqiured_blocks; i++)
            t_print("this block was just freed %x", (yest - bitmap_start) / 0x20000);
    }

    void check_blocks(uint64_t range)
    {
        for(uint64_t i = 0; i < range; i++) {
            if(!isset(i))
                t_print("This block is not allocated: %d", i);
            else
                t_print("This block is allocated: %d", i);
        }
        t_print("First free: %d", first_free());
    }
}

void block_show()
{
    s_print(VGA_GREEN, 0, 0, "PMM: blocks num: %d", total_blocks);
    s_print(VGA_GREEN, 0, 1, "PMM: bitmap addr: %a", (uint64_t)bitmap);
    s_print(VGA_GREEN, 0, 2, "PMM: bitmap size: %d", size);
    s_print(VGA_GREEN, 0, 3, "PMM: addr strat: %a", (uint64_t)block_start);
}
