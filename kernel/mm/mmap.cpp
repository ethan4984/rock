#include <mm/mmap.hpp>
#include <mm/slab.hpp>
#include <fs/fd.hpp>
#include <sched/smp.hpp>
#include <sched/scheduler.hpp>
#include <drivers/hpet.hpp>

namespace mm {

void *mmap(vmm::pmlx_table *page_map, void *addr, size_t length, int prot, int flags, int fd, [[maybe_unused]] ssize_t off) {
    if(length == 0 || length % vmm::page_size) {
        set_errno(einval); 
        return (void*)-1;
    }

    addr = (void*)((uint64_t)addr - ((uint64_t)addr & (vmm::page_size - 1)));

    uintptr_t base;
    length += vmm::page_size;

    if(flags & map_fixed) {
        base = (uintptr_t)addr;
    } else {
        base = page_map->mmap.base;
        page_map->mmap.base += length; 
    }

    vmm::region new_region {};

    new_region.base = base;
    new_region.limit = length;
    new_region.flags = flags;
    new_region.prot = prot;

    page_map->mmap.region_list.push(new_region);

    [&](vmm::pmlx_table *page_map, vmm::region region) -> void {
        auto page_cnt = div_roundup(region.limit, vmm::page_size);
        page_map->map_range(region.base, page_cnt, prot, -1);
    } (page_map, new_region);

    return (void*)new_region.base;
}

ssize_t munmap(vmm::pmlx_table *page_map, void *addr, size_t length) {
    return 0;
}

extern "C" void syscall_mmap(regs *regs_cur) {
    smp::cpu *cpu = smp::core_local();
    regs_cur->rax = (size_t)mmap(cpu->page_map, (void*)regs_cur->rdi, regs_cur->rsi, (int)regs_cur->rdx | (1 << 2), (int)regs_cur->r10, (int)regs_cur->r8, (ssize_t)regs_cur->r9);
}

extern "C" void syscall_munmap(regs *regs_cur) {
    smp::cpu *cpu = smp::core_local();
    regs_cur->rax = (size_t)munmap(cpu->page_map, (void*)regs_cur->rdi, regs_cur->rsi);
}

}
