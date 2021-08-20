#include <drivers/pci.hpp>
#include <drivers/nvme/nvme.hpp>
#include <drivers/ahci/ahci.hpp>
#include <drivers/xhci/xhci.hpp>
#include <drivers/hda/hda.hpp>
#include <int/apic.hpp>
#include <acpi/rsdt.hpp>

#define GET_CLASS(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0x8) >> 24)

#define GET_SUB_CLASS(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0x8) >> 16)

#define GET_PROG_IF(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0x8) >> 8)

#define GET_DEVICE_ID(BUS, DEVICE, FUNC) \
    (uint16_t)(pci::raw_read(BUS, DEVICE, FUNC, 0) >> 16)

#define GET_VENDOR_ID(BUS, DEVICE, FUNC) \
    (uint16_t)(pci::raw_read(BUS, DEVICE, FUNC, 0))

#define GET_HEADER_TYPE(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0xc) >> 16)

#define GET_SECONDARY_BUS(BUS, DEVICE, FUNC) \
    (uint8_t)(pci::raw_read(BUS, DEVICE, FUNC, 0x18) >> 8)

namespace pci {

static inline uint32_t raw_read(uint8_t bus, uint8_t device_code, uint8_t func, uint8_t off) {
    outd(0xcf8, (1 << 31) | // enable
                ((uint32_t)bus << 16) | // bus number
                (((uint32_t)device_code & 31) << 11) | // device number
                (((uint32_t)func & 7) << 8) | // function number
                ((uint32_t)off & ~(3)));
    return ind(0xcfc);
}

static inline void raw_write(uint32_t data, uint8_t bus, uint8_t device_code, uint8_t func, uint8_t off) {
    outd(0xcf8, (1 << 31) | // enable
                ((uint32_t)bus << 16) | // bus number
                (((uint32_t)device_code & 31) << 11) | // device number
                (((uint32_t)func & 7) << 8) | // function number
                ((uint32_t)off & ~(3)));
    outd(0xcfc, data);
}

static void scan_bus(uint8_t bus) {
    for(uint8_t dev = 0; dev < 32; dev++) { 
        if(GET_VENDOR_ID(bus, dev, 0) == 0xffff)
            continue;

        device_list.push(device(bus, dev, 0));

        if(GET_HEADER_TYPE(bus, dev, 0) & (1 << 7)) { // multi function device
            for(uint8_t func = 1; func < 8; func++) 
                if(GET_VENDOR_ID(bus, dev, func) != 0xffff) 
                    device_list.push(device(bus, dev, func));
        }
    }
}

device::device(uint8_t bus, uint8_t device_code, uint8_t func) : bus(bus),
                                                                 device_code(device_code),
                                                                 func(func),
                                                                 class_code(GET_CLASS(bus, device_code, func)),
                                                                 sub_class(GET_SUB_CLASS(bus, device_code, func)),
                                                                 prog_if(GET_PROG_IF(bus, device_code, func)),
                                                                 device_id(GET_DEVICE_ID(bus, device_code, func)),
                                                                 vendor_id(GET_VENDOR_ID(bus, device_code, func)) {
    if((GET_HEADER_TYPE(bus, device_code, func) & ~(1 << 7)) == 1) { // pci to pci bridge
        scan_bus(GET_SECONDARY_BUS(bus, device_code, func));
    }
}

void device::become_bus_master() {
    write<uint16_t>(0x4, read<uint16_t>(0x4) | (1 << 2));    
}

void device::enable_mmio() {
    write<uint16_t>(0x4, read<uint16_t>(0x4) | (1 << 1));
}

int device::get_bar(bar &ret, size_t num) {
    if(GET_HEADER_TYPE(bus, device_code, func) != 0) // ensure hdr type == 0
        return -1;

    if(num > 5) 
        return -1;

    size_t bar_off = 0x10 + num * 4;
    size_t bar_low = read<uint32_t>(bar_off);
    size_t is_mmio = !(bar_low & 1);

    write<uint32_t>(bar_off, ~0);
    size_t bar_size_low = read<uint32_t>(bar_off);
    write<uint32_t>(bar_off, bar_low); 

    if(((bar_low >> 1) & 0b11) == 0b10) { // is 64 bit
        size_t bar_high = read<uint32_t>(bar_off + 4);

        write<uint32_t>(bar_off + 4, ~0);
        size_t bar_size_high = read<uint32_t>(bar_off + 4);
        write<uint32_t>(bar_off + 4, bar_high); 

        size_t size = ((bar_size_high << 32) | bar_size_low) & ~(is_mmio ? 0b1111 : 0b11);
        size = ~size + 1;

        size_t base = ((bar_high << 32) | bar_low) & ~(is_mmio ? 0b1111 : 0b11);

        ret = { base, size };

        return 0;
    }

    size_t size = bar_size_low & is_mmio ? 0b1111 : 0b11;
    size = ~size + 1; 

    ret = { bar_low, size };

    return 0;
}

int device::set_interrupt(uint8_t vec) {
    if(msix_support == true) {
        return set_msix(vec);
    } else if(msi_support == true) {
        return set_msi(vec);
    }
    return -1;
}

int device::set_msi(uint8_t vec) {
    msi::msg_cntl message_control;

    message_control.raw = read<uint16_t>(msi_offset + 2);

    uint32_t reg0 = 0x4;
    uint32_t reg1 = 0x8;

    if(message_control.c64) { // 64 bit support
        reg1 = 0xc;
    } 

    msi::data data;
    msi::address address;

    address.raw = read<uint32_t>(msi_offset + reg1);
    data.raw = read<uint32_t>(msi_offset + reg0);

    data.delivery_mode = 0;
    data.vector = vec;

    address.base_address = 0xfee;
    address.destination_id = apic::lapic->read(apic::lapic->id_reg());

    write<uint32_t>(msi_offset + reg0, address.raw);
    write<uint32_t>(msi_offset + reg1, data.raw);

    message_control.enable = 1;
    message_control.mme = 0;

    write<uint16_t>(msi_offset + 2, message_control.raw);

    return 0;
}

int device::set_msix(uint8_t vec) {
    msix::address table_ptr;
    table_ptr.raw = read<uint32_t>(msix_offset + 4);
    read<uint32_t>(msix_offset + 8);

    auto bar_index = table_ptr.bir;
    auto bar_offset = table_ptr.offset << 3;

    bar table_bar;
    get_bar(table_bar, bar_index);

    auto bar_base = table_bar.base + bar_offset;

    volatile msix::entry *table = (volatile msix::entry*)(bar_base + vmm::high_vma);

    msi::data data {};
    msi::address address {};

    data.delivery_mode = 0;
    data.vector = vec;

    address.base_address = 0xfee;
    address.destination_id = apic::lapic->read(apic::lapic->id_reg());

    auto table_index = msix_table_bitmap.alloc();
    if(table_index == -1)
        return -1;

    msix::vector_control vec_cntl {};
    vec_cntl.mask = 0;

    table[table_index].addr_low = address.raw;
    table[table_index].addr_high = 0;
    table[table_index].data = data.raw;
    table[table_index].control = vec_cntl.raw;

    msix::msg_cntl message_control;

    message_control.raw = read<uint16_t>(msix_offset + 2);
    message_control.enable = 1;
    message_control.mask = 0;
    write<uint16_t>(msix_offset + 2, message_control.raw);

    return 0;
}

void init() {
    for(int bus = 0; bus < 256; bus++) {
        scan_bus(bus);
    }

    for(size_t i = 0; i < device_list.size(); i++) {
        device &pci_device = device_list[i];

        print("[PCI] device: {x}\n", i);
        print("[PCI] \tVendor: {x} on [bus] {x} [function] {x}\n",  pci_device.vendor_id,
                                                                    pci_device.bus,
                                                                    pci_device.func);
        print("[PCI] \tdevice type: [class] {x} [subclass] {x} [prog_IF] {x} [device ID] {x}\n",    pci_device.class_code,
                                                                                                    pci_device.sub_class,
                                                                                                    pci_device.prog_if,
                                                                                                    pci_device.device_id);

        int off = pci_device.read<uint16_t>(0x6) & (1 << 4) ? pci_device.read<uint8_t>(0x34) : -1;

        if(off != -1) {
            while(off) {
                uint8_t id = pci_device.read<uint8_t>(off);

                switch(id) { 
                    case 0x5:
                        pci_device.msi_support = true;
                        pci_device.msi_offset = off;
                        break;
                    case 0x11:
                        pci_device.msix_support = true;
                        pci_device.msix_offset = off;
                        pci_device.msix_table_bitmap = lib::bitmap(2048);
                }

                off = pci_device.read<uint8_t>(off + 1);
            }
        }

        switch(pci_device.class_code) {
            case 1: { // mass storage controller
                switch(pci_device.sub_class) {
                    case 6: { // serial ata
                        new ahci::controller(pci_device);
                        break;
                    }
                    case 8: { // nvme
                        new nvme::controller(pci_device);
                    }
                }
                break;
            }
            case 4: { // Multimedia controller
                switch(pci_device.sub_class) {
                    case 3: // audio
                        new hda::controller(pci_device);
                }
                break;
            }
            case 0xc: { // serial bus controller
                switch(pci_device.sub_class) {
                    case 0x3: // xhci
                        new xhci::controller(pci_device);
                }
            }
        }
    }
}

}
