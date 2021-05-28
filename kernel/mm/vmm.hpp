#ifndef VMM_HPP_
#define VMM_HPP_

#include <cstdint>
#include <cstddef>
#include <utility>

namespace vmm {

constexpr size_t high_vma = 0xffff800000000000;
constexpr size_t kernel_high_vma = 0xffffffff80000000;
constexpr size_t page_size = 0x1000;

class pml4_entry {
public:
    pml4_entry(uint64_t *entry) : entry(entry) { }
    pml4_entry() = default;

    uint64_t vaddr;
    uint64_t paddr;
    
    enum flags {
        p = 1 << 0,
        rw = 1 << 1,
        us = 1 << 2,
        pwt = 1 << 3, 
        pcd = 1 << 4,
        a = 1 << 5,
        nx = 1ull << 63
    };

    bool is_present() const { return *entry & p; }
    bool is_read_write() const { return *entry & rw; }
    bool is_user() const { return *entry & us; }
    bool is_write_through() const { return *entry & pwt; }
    bool is_cache_disabled() const { return *entry & pcd; }
    bool is_access() const { return *entry & a; }
    bool is_no_exec() const { return *entry & nx; }

    void set_present(bool bit) { set_bit(p, bit); }
    void set_read_write(bool bit) { set_bit(rw, bit); }
    void set_user(bool bit) { set_bit(us, bit); }
    void set_write_through(bool bit) { set_bit(pwt, bit); }
    void set_cache_disabled(bool bit) { set_bit(pcd, bit); }
    void set_no_exec(bool bit) { set_bit(nx, bit); }

    uint64_t get_base() { return *entry & ~(0xfff); }
private:
    void set_bit(uint64_t bit, bool value) {
        if(value) {
            *entry |= bit;
        } else {
            *entry &= ~bit;
        }
    }

    uint64_t *entry;
};

class pml3_entry {
public:
    pml3_entry(uint64_t *entry) : entry(entry) { }
    pml3_entry() = default;

    uint64_t vaddr;
    uint64_t paddr;
    
    enum flags {
        p = 1 << 0,
        rw = 1 << 1,
        us = 1 << 2,
        pwt = 1 << 3, 
        pcd = 1 << 4,
        a = 1 << 5,
        d = 1 << 6,
        ps = 1 << 7,
        g = 1 << 8,
        pat = 1 << 12,
        nx = 1ull << 63
    };

    bool is_present() const { return *entry & p; }
    bool is_read_write() const { return *entry & rw; }
    bool is_user() const { return *entry & us; }
    bool is_write_through() const { return *entry & pwt; }
    bool is_cache_disabled() const { return *entry & pcd; }
    bool is_access() const { return *entry & a; }
    bool is_dirty() const { return *entry & d; }
    bool is_huge() const { return *entry & ps; }
    bool is_global() const { return *entry & g; }
    bool is_pat() const { return *entry & pat; }
    bool is_no_exec() const { return *entry & nx; }

    void set_present(bool bit) { set_bit(p, bit); }
    void set_read_write(bool bit) { set_bit(rw, bit); }
    void set_user(bool bit) { set_bit(us, bit); }
    void set_write_through(bool bit) { set_bit(pwt, bit); }
    void set_cache_disabled(bool bit) { set_bit(pcd, bit); }
    void set_huge(bool bit) { set_bit(ps, bit); }
    void set_global(bool bit) { set_bit(g, bit); }
    void set_pat(bool bit) { set_bit(pat, bit); }
    void set_no_exec(bool bit) { set_bit(nx, bit); }

    uint64_t get_base() { return *entry & ~(0xfff); }
private:
    void set_bit(uint64_t bit, bool value) {
        if(value) {
            *entry |= bit;
        } else {
            *entry &= ~bit;
        }
    }

    uint64_t *entry;
};

class pml2_entry {
public:
    pml2_entry(uint64_t *entry) : entry(entry) { }
    pml2_entry() = default;

    uint64_t vaddr;
    uint64_t paddr;
    
    enum flags {
        p = 1 << 0,
        rw = 1 << 1,
        us = 1 << 2,
        pwt = 1 << 3, 
        pcd = 1 << 4,
        a = 1 << 5,
        d = 1 << 6,
        ps = 1 << 7,
        g = 1 << 8,
        pat = 1 << 12,
        nx = 1ull << 63
    };

    bool is_present() const { return *entry & p; }
    bool is_read_write() const { return *entry & rw; }
    bool is_user() const { return *entry & us; }
    bool is_write_through() const { return *entry & pwt; }
    bool is_cache_disabled() const { return *entry & pcd; }
    bool is_access() const { return *entry & a; }
    bool is_dirty() const { return *entry & d; }
    bool is_huge() const { return *entry & ps; }
    bool is_global() const { return *entry & g; }
    bool is_pat() const { return *entry & pat; }
    bool is_no_exec() const { return *entry & nx; }

    void set_present(bool bit) { set_bit(p, bit); }
    void set_read_write(bool bit) { set_bit(rw, bit); }
    void set_user(bool bit) { set_bit(us, bit); }
    void set_write_through(bool bit) { set_bit(pwt, bit); }
    void set_cache_disabled(bool bit) { set_bit(pcd, bit); }
    void set_huge(bool bit) { set_bit(ps, bit); }
    void set_global(bool bit) { set_bit(g, bit); }
    void set_pat(bool bit) { set_bit(pat, bit); }
    void set_no_exec(bool bit) { set_bit(nx, bit); }

    uint64_t get_base() { return *entry & ~(0xfff); }
private:
    void set_bit(uint64_t bit, bool value) {
        if(value) {
            *entry |= bit;
        } else {
            *entry &= ~bit;
        }
    }

    uint64_t *entry;
};

class pml1_entry {
public:
    pml1_entry(uint64_t *entry) : entry(entry) { }
    pml1_entry() = default;

    uint64_t vaddr;
    uint64_t paddr;
    
    enum flags {
        p = 1 << 0,
        rw = 1 << 1,
        us = 1 << 2,
        pwt = 1 << 3, 
        pcd = 1 << 4,
        a = 1 << 5,
        d = 1 << 6,
        pat = 1 << 7,
        g = 1 << 8,
        nx = 1ull << 63
    };

    bool is_present() const { return *entry & p; }
    bool is_read_write() const { return *entry & rw; }
    bool is_user() const { return *entry & us; }
    bool is_write_through() const { return *entry & pwt; }
    bool is_cache_disabled() const { return *entry & pcd; }
    bool is_access() const { return *entry & a; }
    bool is_dirty() const { return *entry & d; }
    bool is_pat() const { return *entry & pat; }
    bool is_global() const { return *entry & g; }
    bool is_no_exec() const { return *entry & nx; }

    void set_present(bool bit) { set_bit(p, bit); }
    void set_read_write(bool bit) { set_bit(rw, bit); }
    void set_user(bool bit) { set_bit(us, bit); }
    void set_write_through(bool bit) { set_bit(pwt, bit); }
    void set_cache_disabled(bool bit) { set_bit(pcd, bit); }
    void set_pat(bool bit) { set_bit(pat, bit); }
    void set_global(bool bit) { set_bit(g, bit); }
    void set_no_exec(bool bit) { set_bit(nx, bit); }

    uint64_t get_base() { return *entry & ~(0xfff); }
private:
    void set_bit(uint64_t bit, bool value) {
        if(value) {
            *entry |= bit;
        } else {
            *entry &= ~bit;
        }
    }

    uint64_t *entry;
};

class virtual_address {
public:
    virtual_address(uint64_t *pml4_raw, uint64_t vaddr, uint64_t paddr,
                    uint64_t pml4_flags, uint64_t pml3_flags,
                    uint64_t pml2_flags, uint64_t pml1_flags);
    virtual_address(uint64_t *pml4_raw, uint64_t vaddr);
    virtual_address() = default;

    uint64_t unmap();

    pml4_entry pml4e;
    pml3_entry pml3e;
    pml2_entry pml2e;
    pml1_entry pml1e;

    uint64_t page_length;
private:
    void compute_indices() {
        pml4_idx = (vaddr >> 39) & 0x1ff;
        pml3_idx = (vaddr >> 30) & 0x1ff;
        pml2_idx = (vaddr >> 21) & 0x1ff;
        pml1_idx = (vaddr >> 12) & 0x1ff;
    }

    uint64_t vaddr;
    uint64_t paddr;

    size_t pml4_idx; 
    size_t pml3_idx;
    size_t pml2_idx;
    size_t pml1_idx;

    uint64_t *pml4_raw;
    uint64_t *pml3_raw;
    uint64_t *pml2_raw;
    uint64_t *pml1_raw;
};

struct page_table {
    page_table(uint64_t *pml4) : pml4_raw(pml4), lock(0) { }
    page_table();

    void map_range(uint64_t vaddr, size_t cnt, size_t flags);
    void unmap_range(uint64_t vaddr, size_t cnt);

    void map_page(uint64_t vaddr, uint64_t flags);
    void unmap_page(uint64_t vaddr);

    void init() {
        asm volatile ("movq %0, %%cr3" :: "r" (reinterpret_cast<uint64_t>(pml4_raw) - high_vma) : "memory");
    }
    
    uint64_t *pml4_raw;
    size_t lock;
};

inline uint64_t get_pml4() {
    uint64_t pml4;
    asm volatile ("mov %%cr3, %0" : "=r"(pml4));
    return pml4;
}

inline void tlb_flush() {
    asm volatile ("mov %0, %%cr3" :: "r" (get_pml4()) : "memory");
}

void init();

inline page_table kernel_mapping;

}

#endif
