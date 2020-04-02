#include <process.h>
#include <paging.h>
#include <shitio.h>
#include <memory.h>

#include <stdint.h>
#include <stddef.h>

using namespace standardout;
using namespace MM;

process::process(uint64_t range, void (*entry)()) : num_of_blocks(range), entry_point(entry)
{
    process_begin = (uint64_t*)pagalloc(range);

    uint64_t blocks = 0;
    while(++blocks*0x20000 < range);

    t_print("New process allocated at %x\n", (uint64_t)process_begin);
    s_print(VGA_GREEN, 50, grab_current_y() + 1, "New process requesting %d pages", blocks);

    for(uint64_t i = 0; i < range/0x80; i++) /* process allocator uses blocks of 128 bytes */
        pmem_map.available_blocks[i] = 1;
}

process::~process()
{
    process_begin = 0;
    entry_point = 0;
}

process::process()
{

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

    t_print("Process %x freed\n", (uint64_t)ref.process_begin);
    pagfree(ref.process_begin, ref.num_of_blocks);
    ref.process_begin = NULL;
}

uint64_t process::first_freep()
{
    for(uint64_t i = 0; i < num_of_blocks; i++) {
        if(pmem_map.available_blocks[i] == 1)
            return i;
    }

    t_print("ok: you are out of blocks, this process will probably break");
    return num_of_blocks + 1;
}

uint64_t process::allocate_pblock()
{
    uint64_t new_pblock = first_freep();
    pmem_map.available_blocks[new_pblock] = 2;
    return new_pblock;
}

void process::free_pblock(uint64_t index)
{
    pmem_map.available_blocks[index] = 1;
}

void *process::pmalloc(size_t size)
{
    t_print("pmalloc in process\n");

    if(!size) {
        t_print("PMALLOC: stop trying to allocate blocks of zero size");
        return 0;
    }

    uint64_t reqiured_blocks = 0;

    bool is_one = true;

    while(++reqiured_blocks*0x80 < size)
        is_one = false;

    uint64_t freed = first_freep();

    if(is_one) {
        t_print("\n\tstatus: single block\n\treqiured blocks: %d\n", reqiured_blocks);
        if(first_freep() == num_of_blocks + 1) {
            t_print("PMALLOC: we ran out of blocks bruh");
            return 0;
        }

        allocate_pblock();

        t_print("\npmalloc finished : returning %x\n", block_start + freed*0x80);
        return block_start + freed*0x80;
    }

    uint64_t i;
    t_print("\n\tstatus: multi block\n\treqiured blocks: %d\n", reqiured_blocks);

    for(i = 0; i < reqiured_blocks; i++) {
        if(allocate_pblock() == num_of_blocks + 1) {
            t_print("BRUH: we ran out of blocks bruh");
            return 0;
        }
    }
    t_print("\npmalloc allocation finished : returning %x\n", block_start + (freed*i)*0x80);
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

    uint64_t reqiured_blocks = 0;
    while(++reqiured_blocks*0x80 < size);

    if(reqiured_blocks == 1) {
        free_pblock(((uint64_t)location - (uint64_t)process_begin) / 0x80);
        t_print("This process blokc was just freed %d\n", ((uint64_t)location - (uint64_t)process_begin) / 0x80);
        return;
    }

    free_pblock((uint64_t)location - (uint64_t)process_begin / 0x80);
    for(uint64_t i = 0; i < reqiured_blocks; i++)
        t_print("This process block was just freed %x", (uint64_t)location - (uint64_t )process_begin);
    t_print("\n");
}

void process::show_blocks(uint64_t range)
{
    for(uint64_t i = 0; i < range; i++) {
        t_print("%d\n", pmem_map.available_blocks[i]);
    }
}

extern void back_state(void) asm("back_state");
extern void new_state(void) asm("new_state");
extern void restore_state(void) asm("restore_state");

void process::save_regs()
{
    asm volatile ("movq %%rax, %0" : "=r"(rax));
    asm volatile ("movq %%rbx, %0" : "=r"(rbx));
    asm volatile ("movq %%rcx, %0" : "=r"(rcx));
    asm volatile ("movq %%rdx, %0" : "=r"(rdx));

    asm volatile ("movq %%rsi, %0" : "=r"(rsi));
    asm volatile ("movq %%rdi, %0" : "=r"(rdi));
    asm volatile ("movq %%rbp, %0" : "=r"(rbp));
    asm volatile ("movq %%rsp, %0" : "=r"(rsp));

    asm volatile ("movq %%r8, %0" : "=r"(r8));
    asm volatile ("movq %%r9, %0" : "=r"(r9));
    asm volatile ("movq %%r10, %0" : "=r"(r10));
    asm volatile ("movq %%r11, %0" : "=r"(r11));
    asm volatile ("movq %%r12, %0" : "=r"(r12));
    asm volatile ("movq %%r13, %0" : "=r"(r13));
    asm volatile ("movq %%r14, %0" : "=r"(r14));
    asm volatile ("movq %%r15, %0" : "=r"(r15));
}

void process::restore()
{
    /*TODO*/
}
