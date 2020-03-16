#include <process.h>
#include <paging.h>
#include <shitio.h>
#include <memory.h>

#include <stdint.h>
#include <stddef.h>

using namespace standardout;
using namespace MM;

process::process(size_t range) : num_of_blocks(range)
{
    process_begin = (uint32_t*)malloc(range);
    t_print("New process allocated at %x\n", (uint32_t)process_begin);

    for(uint32_t i = 0; i < range/0x80; i++) /* process allocator uses blocks of 128 bytes */
        pmem_map.available_blocks[i] = 1;
}

bool process::null_check()
{
    return (process_begin != NULL) ? true : false;
}

void free_process(process &ref)
{
    if(!ref.null_check()) {
        t_print("Bruh : broken process : NULL");
        return;
    }

    t_print("Process %x freed\n", (uint32_t)ref.process_begin);
    free(ref.process_begin, ref.num_of_blocks);
    ref.process_begin = NULL;
}

uint32_t process::first_freep()
{
    for(int i = 0; i < num_of_blocks; i++) {
        if(pmem_map.available_blocks[i] == 1)
            return i;
    }

    t_print("ok: you are out of blocks, this process will probably break");
    return 0;
}

uint32_t process::allocate_pblock()
{
    uint32_t new_pblock = first_freep();
    pmem_map.available_blocks[new_pblock] = 2;
    return new_pblock;
}

void process::free_pblock(size_t index)
{
    pmem_map.available_blocks[index] = 1;
}

void *process::pmalloc(size_t size)
{
    t_print("Process block allocation in process\n");

    if(!size) {
        t_print("\tBRUH: stop trying to allocate blocks of zero size");
        return 0;
    }

    uint32_t reqiured_blocks = 0;

    bool is_one = true;

    while(++reqiured_blocks*0x80 < size)
        is_one = false;

    uint32_t freed = first_freep();

    t_print("%x", freed);

    if(is_one) {
        if(first_freep() == static_cast<unsigned int>(-69)) {
            t_print("BRUH: we ran out of blocks bruh");
            return 0;
        }

        allocate_pblock();

        t_print("\nProcess block allocation finished : returning %x\n", block_start + freed*0x80);

        return block_start + freed*0x80;
    }

    uint32_t i;

    for(i = 0; i < reqiured_blocks; i++) {
        if(allocate_pblock() == static_cast<unsigned int>(-69))
            t_print("BRUH: we ran out of blocks bruh");
    }
    t_print("\nProcess block allocation finished : returning %x\n", block_start + (freed*i)*0x80);
    return block_start + (freed*i)*0x80;
}

void process::pfree(void *location, size_t size)
{
    if(location == NULL) {
        t_print("stop trying to free null memory");
        return;
    }

    if(location <= process_begin && location >= process_begin + (num_of_blocks*0x4000)) {
        t_print("Bruh your out of bounds");
        return;
    }

    unsigned long int reqiured_blocks = 0;
    while(++reqiured_blocks*0x80 < size);

    if(reqiured_blocks == 1) {
        free_pblock((uint32_t)location - (uint32_t)process_begin / 0x80);
        t_print("This process blokc was just freed %x", (uint32_t)location - (uint32_t)process_begin);
        return;
    }

    free_pblock((uint32_t)location - (uint32_t)process_begin / 0x80);
    for(unsigned long int i = 0; i < reqiured_blocks; i++)
        t_print("This process block was just freed %x", (uint32_t)location - (uint32_t )process_begin);
}






