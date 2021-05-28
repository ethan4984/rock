#include <mm/pmm.hpp>

namespace vmm {

virtual_address::virtual_address(uint64_t *pml4_raw, uint64_t vaddr, uint64_t paddr,
                                 uint64_t pml4_flags, uint64_t pml3_flags,
                                 uint64_t pml2_flags, uint64_t pml1_flags) : vaddr(vaddr), paddr(paddr), pml4_raw(pml4_raw) {
    compute_indices();

    if(!pml4_raw[pml4_idx]) {
        if(!pml4_flags)
            return;
        pml4_raw[pml4_idx] = pmm::calloc(1) | pml4_flags;
    }

    pml4e = pml4_entry(&pml4_raw[pml3_idx]);
    pml3_raw = reinterpret_cast<uint64_t*>((pml4_raw[pml4_idx] & ~(0xfff)) + high_vma);

    if(!pml3_raw[pml3_idx]) {
        if(!pml3_flags)
            return;

        if(pml3_flags & pml3_entry::flags::ps) {
            pml3_raw[pml3_idx] = paddr | pml3_flags; 
        } else {
            pml3_raw[pml3_idx] = pmm::calloc(1) | pml3_flags; 
        }
    }

    pml3e = pml3_entry(&pml3_raw[pml3_idx]);
    if(pml3e.is_huge()) {
        page_length = 0x40000000;
        return;
    }

    pml2_raw = reinterpret_cast<uint64_t*>((pml3_raw[pml3_idx] & ~(0xfff)) + high_vma);

    if(!pml2_raw[pml2_idx]) {
        if(!pml2_flags)
            return;

        if(pml2_flags & pml2_entry::flags::ps) {
            pml2_raw[pml2_idx] = paddr | pml2_flags; 
        } else {
            pml2_raw[pml2_idx] = pmm::calloc(1) | pml2_flags; 
        }
    }

    pml2e = pml2_entry(&pml2_raw[pml2_idx]);
    if(pml2e.is_huge()) {
        page_length = 0x200000;
        return;
    }

    pml1_raw = reinterpret_cast<uint64_t*>((pml2_raw[pml2_idx] & ~(0xfff)) + high_vma);

    if(!pml1_raw[pml1_idx]) {
        if(!pml1_flags)
            return;

        pml1_raw[pml1_idx] = paddr | pml1_flags;
    }

    pml1e = pml1_entry(&pml1_raw[pml1_idx]);
    page_length = 0x1000;
}

uint64_t virtual_address::unmap() {
    if(pml3e.is_huge()) {
        uint64_t paddr = pml3e.get_base();
        pml3e.set_present(0);
        return paddr;
    } 

    if(pml2e.is_huge()) {
        uint64_t paddr = pml2e.get_base();
        pml2e.set_present(0);
        return paddr;
    }

    uint64_t paddr = pml1e.get_base();
    pml1e.set_present(0);
    return paddr;
}

virtual_address::virtual_address(uint64_t *pml4_raw, uint64_t vaddr) : vaddr(vaddr), pml4_raw(pml4_raw) {
    compute_indices();

    if(!pml4_raw[pml4_idx]) {
        return;
    }

    pml4e = pml4_entry(&pml4_raw[pml4_idx]);
    pml3_raw = reinterpret_cast<uint64_t*>((pml4_raw[pml4_idx] & ~(0xfff)) + high_vma);

    if(!pml3_raw[pml3_idx]) {
        return;
    }

    pml3e = pml3_entry(&pml3_raw[pml3_idx]);
    if(pml3e.is_huge())
        return;

    pml2_raw = reinterpret_cast<uint64_t*>((pml3_raw[pml3_idx] & ~(0xfff)) + high_vma);

    if(!pml2_raw[pml2_idx]) {
        return;
    }

    pml2e = pml2_entry(&pml2_raw[pml2_idx]);
    if(pml2e.is_huge())
        return;

    pml1_raw = reinterpret_cast<uint64_t*>((pml2_raw[pml2_idx] & ~(0xfff)) + high_vma);

    if(!pml1_raw[pml1_idx]) {
        return;
    }

    pml1e = pml1_entry(&pml1_raw[pml3_idx]);
}

page_table::page_table() : lock(0) {
    pml4_raw = reinterpret_cast<uint64_t*>(pmm::calloc(1) + high_vma);
    pml4_raw[256] = reinterpret_cast<uint64_t*>(get_pml4() + high_vma)[256];
    pml4_raw[511] = reinterpret_cast<uint64_t*>(get_pml4() + high_vma)[511];
}

void page_table::map_range(uint64_t vaddr, size_t cnt, size_t flags) {
    spin_lock(&lock);

    if(flags & (1 << 7)) {
        for(size_t i = 0; i < cnt; i++) {
            virtual_address(pml4_raw, vaddr, pmm::calloc(0x200),
                            pml4_entry::flags::p | pml4_entry::flags::rw,
                            pml3_entry::flags::p | pml3_entry::flags::rw,
                            flags,
                            0);
            vaddr += 0x200000;
        }
    } else {
        for(size_t i = 0; i < cnt; i++) {
            virtual_address(pml4_raw, vaddr, pmm::calloc(1),
                            pml4_entry::flags::p | pml4_entry::flags::rw,
                            pml3_entry::flags::p | pml3_entry::flags::rw,
                            pml2_entry::flags::p | pml2_entry::flags::rw,
                            flags);
            vaddr += 0x1000;
        }
    }

    spin_release(&lock);
}

void page_table::unmap_range(uint64_t vaddr, size_t cnt) {
    spin_lock(&lock);

    for(size_t i = 0; i < cnt; i++) {
        virtual_address addr(pml4_raw, vaddr);
        if(!addr.page_length) {
            spin_release(&lock);
            return;
        }
        addr.unmap();
        vaddr += addr.page_length;
    }
    spin_release(&lock);
}

void page_table::map_page(uint64_t vaddr, uint64_t flags) {
    spin_lock(&lock);

    if(flags & (1 << 7)) {
        virtual_address(pml4_raw, vaddr, pmm::calloc(0x200),
                        pml4_entry::flags::p | pml4_entry::flags::rw,
                        pml3_entry::flags::p | pml3_entry::flags::rw,
                        flags,
                        0);
    } else {
        virtual_address(pml4_raw, vaddr, pmm::calloc(1),
                        pml4_entry::flags::p | pml4_entry::flags::rw,
                        pml3_entry::flags::p | pml3_entry::flags::rw,
                        pml2_entry::flags::p | pml2_entry::flags::rw,
                        flags);
    }

    spin_release(&lock);

    tlb_flush();
}

void page_table::unmap_page(uint64_t vaddr) {
    spin_lock(&lock);

    virtual_address addr(pml4_raw, vaddr);

    if(!addr.page_length) {
        spin_release(&lock);
        return;
    }

    addr.unmap();

    spin_release(&lock);
}

void init() {
    uint64_t *pml4_raw = reinterpret_cast<uint64_t*>(pmm::calloc(1) + high_vma);

    size_t phys = 0;
    for(size_t i = 0; i < 0x200; i++) {
        virtual_address(pml4_raw, phys + kernel_high_vma, phys,
                        pml4_entry::flags::p | pml4_entry::flags::rw,
                        pml3_entry::flags::p | pml3_entry::flags::rw,
                        pml2_entry::flags::p | pml2_entry::flags::rw | pml2_entry::flags::ps | pml2_entry::flags::g,
                        0);
        phys += 0x200000;
    }

    phys = 0;
    for(size_t i = 0; i < pmm::total_mem / 0x200000; i++) {
        virtual_address(pml4_raw, phys + high_vma, phys,
                        pml4_entry::flags::p | pml4_entry::flags::rw,
                        pml3_entry::flags::p | pml3_entry::flags::rw,
                        pml2_entry::flags::p | pml2_entry::flags::rw | pml2_entry::flags::ps | pml2_entry::flags::g,
                        0);
        phys += 0x200000;
    }

    kernel_mapping = page_table(pml4_raw);

    asm volatile ("movq %0, %%cr3" :: "r" (reinterpret_cast<uint64_t>(pml4_raw) - high_vma) : "memory");
}

}
