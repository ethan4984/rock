#ifndef PCI_HPP_
#define PCI_HPP_

#include <vector.hpp>
#include <acpi/rsdt.hpp>
#include <bitmap.hpp>

namespace pci {

namespace msi {

union [[gnu::packed]] address {
    struct {
        uint32_t reserved0 : 2;
        uint32_t destination_mode : 1;
        uint32_t redirection_hint : 1;
        uint32_t reserved_0 : 8;
        uint32_t destination_id : 8;
        uint32_t base_address : 12;
    };
    uint32_t raw;
};

union [[gnu::packed]] data {
    struct {
        uint32_t vector : 8;
        uint32_t delivery_mode : 3;
        uint32_t reserved : 3;
        uint32_t level : 1;
        uint32_t trigger_mode : 1;
        uint32_t reserved0 : 16;
    };
    uint32_t raw;
};

union [[gnu::packed]] msg_cntl {
    struct {
        uint32_t enable : 1;
        uint32_t mmc : 3;
        uint32_t mme : 3;
        uint32_t c64 : 1;
        uint32_t pvm : 1;
        uint32_t reserved1 : 7;
    };
    uint32_t raw;
};

}

namespace msix {

union [[gnu::packed]] msg_cntl {
    struct [[gnu::packed]] {
        uint16_t table : 11;
        uint16_t reserved : 3;
        uint16_t mask : 1;
        uint16_t enable : 1;
    };
    uint16_t raw;
};

union [[gnu::packed]] address {
    struct {
        uint32_t bir : 3;
        uint32_t offset : 29;
    };
    uint32_t raw;
};

union [[gnu::packed]] vector_control {
    struct {
        uint32_t mask : 1; 
        uint32_t reserved : 31;
    };
    uint32_t raw;
};

struct [[gnu::packed]] entry {
    uint32_t addr_low; 
    uint32_t addr_high;
    uint32_t data;
    uint32_t control;
};

}

inline struct [[gnu::packed]] mcfg {
    uint32_t signature;
    acpi::hdr hdr_ptr;
    uint64_t reserved;

    struct [[gnu::packed]] {
        uint64_t base;
        uint16_t segment_group;
        uint8_t start_bus;
        uint8_t end_bus;
        uint32_t reserved;
    } alloactions[];
} *mcfg_ptr;

struct bar {
    size_t base;
    size_t size;
};

struct device {
    device(uint8_t bus, uint8_t device, uint8_t func);
    device() = default;

    template <typename T>
    void write(uint8_t off, T data) {
        outd(0xcf8, (1 << 31) | // enable
                    ((uint32_t)bus << 16) | // bus number
                    (((uint32_t)device_code & 31) << 11) | // device number
                    (((uint32_t)func & 7) << 8) | // function number
                    ((uint32_t)off & ~(3)));

        switch(sizeof(T)) {
            case 4:
                outd(0xcfc + (off & 3), data);
                break;
            case 2:
                outw(0xcfc + (off & 3), data);
                break;
            case 1:
                outb(0xcfc + (off & 3), data);
        }
    }

    template <typename T> 
    T read(uint8_t off) {
        outd(0xcf8, (1 << 31) | // enable
                    ((uint32_t)bus << 16) | // bus number
                    (((uint32_t)device_code & 31) << 11) | // device number
                    (((uint32_t)func & 7) << 8) | // function number
                    ((uint32_t)off & ~(3)));

        switch(sizeof(T)) {
            case 4:
                return ind(0xcfc + (off & 3));
            case 2:
                return inw(0xcfc + (off & 3));
            case 1:
                return inb(0xcfc + (off & 3));
            default:
                return -1;
        }
    }

    void become_bus_master();
    void enable_mmio();
    int set_interrupt(uint8_t vec);
    int set_msi(uint8_t vec);
    int set_msix(uint8_t vec);
    int get_bar(bar &ret, size_t num);

    uint8_t bus;
    uint8_t device_code;
    uint8_t func;
    uint8_t class_code;
    uint8_t sub_class;
    uint8_t prog_if;
    uint16_t device_id;
    uint16_t vendor_id;
    uint32_t read(uint8_t off);

    friend void init(); 
private:
    uint32_t raw_read(uint8_t bus, uint8_t device_code, uint8_t func, uint8_t off);
    void raw_write(uint32_t data, uint8_t bus, uint8_t device_code, uint8_t func, uint8_t off);

    lib::bitmap msix_table_bitmap;

    size_t msi_offset;
    size_t msix_offset;

    bool msix_support;
    bool msi_support;
};

inline lib::vector<device> device_list;

void init();

}

#endif
