#include <drivers/pci.h>
#include <debug.h>
#include <vec.h>

struct pci_device *pci_devices;
uint64_t pci_device_cnt;

static void add_pci_device(struct pci_device new_device) {
    if(pci_device_cnt + 1 % 10 == 0) {
        pci_devices = krealloc(pci_devices, pci_device_cnt + 10);
    }
    pci_devices[pci_device_cnt++] = new_device;
}

static void check_device(uint8_t bus, uint8_t device, uint8_t function) {
    int is_bridge = 1;

    if(((uint8_t)(pci_read(bus, device, function, 0xC) >> 16) & ~(1 << 7)) != 1 || (uint8_t)(pci_read(bus, device, function, 0x8) >> 24) != 6 || (uint8_t)(pci_read(bus, device, function, 0x8) >> 16) != 4)
        is_bridge = 0;

    if(is_bridge) {
        pci_scan_bus((uint8_t)(pci_read(bus, device, function, 0x18) >> 8));
    } else {
        add_pci_device((struct pci_device) { (uint8_t)(pci_read(bus, device, function, 0x8) >> 24), // class 
                                             (uint8_t)(pci_read(bus, device, function, 0x8) >> 16), // subclass
                                             (uint8_t)(pci_read(bus, device, function, 0x8) >> 8), // prog_IF
                                             (uint16_t)(pci_read(bus, device, function, 0) >> 16), // device ID
                                             (uint16_t)pci_read(bus, device, function, 0), // vendor ID
                                             bus,
                                             device,
                                             function
                                           });
    }
}

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
    outd(0xcf8, (1 << 31) | ((uint32_t)bus << 16) | (((uint32_t)device & 31) << 11) | (((uint32_t)func & 7) << 8) | ((uint32_t)reg & ~(3)));
    return ind(0xcfc);
}

void pci_write(uint32_t data, uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
    outd(0xcf8, (1 << 31) | ((uint32_t)bus << 16) | (((uint32_t)device & 31) << 11) | (((uint32_t)func & 7) << 8) | ((uint32_t)reg & ~(3)));
    outd(0xcfc, data);
}

void pci_scan_bus(uint8_t bus) {
    for(uint8_t device = 0; device < 32; device++) {
        if((uint16_t)pci_read(bus, device, 0, 0) != 0xffff) {
            check_device(bus, device, 0);
            
            if((uint8_t)(pci_read(bus, device, 0, 0xc) >> 16) & (1 << 7)) {
                for(uint8_t function = 1; function < 8; function++) {
                    if((uint16_t)pci_read(bus, device, function, 0) != 0xffff)
                        check_device(bus, device, function);
                }
            }
        }
    }
}

struct pci_bar pci_get_bar(struct pci_device device, uint64_t bar_num) {
    uint32_t base = pci_read(device.bus, device.device, device.function, 0x10 + (bar_num * 4));

    pci_write(0xffffffff, device.bus, device.device, device.function, 0x10 + (bar_num * 4));
    uint32_t size = pci_read(device.bus, device.device, device.function, 0x10 + (bar_num * 4));
    pci_write(base, device.bus, device.device, device.function, 0x10 + (bar_num * 4));
    
    return (struct pci_bar) { base, ~(size) + 1 };
}

void pci_init() {
    pci_devices = kmalloc(sizeof(struct pci_device) * 10);

    pci_scan_bus(0);
    show_pci_devices();
}

void show_pci_devices() {
    for(uint64_t i = 0; i < pci_device_cnt; i++) {
        kprintf("[PCI] device: %x\n", i);
        kprintf("[PCI] \tVendor: %x on [bus] %x [function] %x\n", pci_devices[i].vendor_ID, pci_devices[i].bus, pci_devices[i].function);
        kprintf("[PCI] \tdevice type: [class] %x [subclass] %x [prog_IF] %x [device ID] %x\n", pci_devices[i].class_code, pci_devices[i].sub_class, pci_devices[i].prog_IF, pci_devices[i].device_ID);
    }
    kprintf("[PCI] total devices: %x\n", pci_device_cnt);
}
