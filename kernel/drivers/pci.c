#include <drivers/pci.h>
#include <drivers/ahci.h>
#include <drivers/e1000.h>
#include <debug.h>
#include <vec.h>

global_vec(pci_device_list);

#define PCI_GET_CLASS(BUS, DEVICE, FUNC) \
    (uint8_t)(pci_raw_read(BUS, DEVICE, FUNC, 0x8) >> 24)

#define PCI_GET_SUB_CLASS(BUS, DEVICE, FUNC) \
    (uint8_t)(pci_raw_read(BUS, DEVICE, FUNC, 0x8) >> 16)

#define PCI_GET_PROG_IF(BUS, DEVICE, FUNC) \
    (uint8_t)(pci_raw_read(BUS, DEVICE, FUNC, 0x8) >> 8)

#define PCI_GET_DEVICE_ID(BUS, DEVICE, FUNC) \
    (uint16_t)(pci_raw_read(BUS, DEVICE, FUNC, 0) >> 16)

#define PCI_GET_VENDOR_ID(BUS, DEVICE, FUNC) \
    (uint16_t)(pci_raw_read(BUS, DEVICE, FUNC, 0))

#define PCI_GET_HEADER_TYPE(BUS, DEVICE, FUNC) \
    (uint8_t)(pci_raw_read(BUS, DEVICE, FUNC, 0xc) >> 16)

#define PCI_GET_SECONDARY_BUS(BUS, DEVICE, FUNC) \
    (uint8_t)(pci_raw_read(BUS, DEVICE, FUNC, 0x18) >> 8)

static uint32_t pci_raw_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t off) {
    outd(0xcf8, (1 << 31) | ((uint32_t)bus << 16) | (((uint32_t)device & 31) << 11) | (((uint32_t)function & 7) << 8) | ((uint32_t)off & ~(3)));
    return ind(0xcfc);
}

static void pci_raw_write(uint32_t data, uint8_t bus, uint8_t device, uint8_t function, uint8_t off) {
    outd(0xcf8, (1 << 31) | ((uint32_t)bus << 16) | (((uint32_t)device & 31) << 11) | (((uint32_t)function & 7) << 8) | ((uint32_t)off & ~(3)));
    outd(0xcfc, data);
}

uint32_t pci_read(struct pci_device *device, uint8_t off) {
    outd(0xcf8, (1 << 31) | // enable
                ((uint32_t)device->bus << 16) | // bus number
                (((uint32_t)device->device & 31) << 11) | // device number
                (((uint32_t)device->func & 7) << 8) | // function number
                ((uint32_t)off & ~(3)));
    return ind(0xcfc);
}

void pci_write(struct pci_device *device, uint8_t off, uint32_t data) {
    outd(0xcf8, (1 << 31) | // enable
                ((uint32_t)device->bus << 16) | // bus number
                (((uint32_t)device->device & 31) << 11) | // device number
                (((uint32_t)device->func & 7) << 8) | // function number
                ((uint32_t)off & ~(3)));
    outd(0xcfc, data);
}

void pci_become_bus_master(struct pci_device *device) {
    pci_write(device, 0x4, pci_read(device, 0x4) | (1 << 2));    
}

void pci_scan_bus(uint8_t bus) {
    for(uint8_t device = 0; device < 32; device++) { 
        if(PCI_GET_VENDOR_ID(bus, device, 0) == 0xffff)
            continue;

        pci_scan_device(bus, device, 0);

        if(PCI_GET_HEADER_TYPE(bus, device, 0) & (1 << 7)) { // multi function device
            for(uint8_t func = 1; func < 8; func++) {
                if(PCI_GET_VENDOR_ID(bus, device, func) != 0xffff)
                    pci_scan_device(bus, device, func);
            }
        }
    }
}

void pci_scan_device(uint8_t bus, uint8_t device, uint8_t func) {
    if((PCI_GET_HEADER_TYPE(bus, device, func) & ~(1 << 7)) == 1) { // pci to pci bridge
        pci_scan_bus(PCI_GET_SECONDARY_BUS(bus, device, func)); 
    }

    struct pci_device new_device = {    .class_code = PCI_GET_CLASS(bus, device, func),
                                        .sub_class = PCI_GET_SUB_CLASS(bus, device, func),
                                        .prog_if = PCI_GET_PROG_IF(bus, device, func),
                                        .device_id = PCI_GET_DEVICE_ID(bus, device, func),
                                        .vendor_id = PCI_GET_VENDOR_ID(bus, device, func),
                                        .bus = bus,
                                        .device = device,
                                        .func = func
                                   };

    vec_push(struct pci_device, pci_device_list, new_device); 
}

int pci_get_bar(struct pci_device *device, struct pci_bar *ret, size_t bar_num) {
    if(PCI_GET_HEADER_TYPE(device->bus, device->device, device->func) != 0) // ensure header type == 0
        return -1;

    size_t bar_off = 0x10 + bar_num * sizeof(uint32_t);

    ret->base = pci_read(device, bar_off);

    pci_write(device, bar_off, ~0);
    ret->size = pci_read(device, bar_off);
    pci_write(device, bar_off, ret->base);

    return 0;
}

void pci_scan() { 
    for(int bus = 0; bus < 256; bus++) {
        pci_scan_bus(bus);
    }

    for(size_t i = 0; i < pci_device_list.element_cnt; i++) {
        struct pci_device *device = vec_search(struct pci_device, pci_device_list, i);

        kprintf("[PCI] device: %x\n", i);
        kprintf("[PCI] \tVendor: %x on [bus] %x [function] %x\n", device->vendor_id, device->bus, device->func);
        kprintf("[PCI] \tdevice type: [class] %x [subclass] %x [prog_IF] %x [device ID] %x\n", device->class_code, device->sub_class, device->prog_if, device->device_id);

        switch(device->class_code) { 
            case 1: // mass storage controller
                switch(device->sub_class) {
                    case 6: // serial ata
                        ahci_init(device);
                        break;
                }
                break;
            case 0x2: // network controller
                switch(device->sub_class) {
                    case 0: // ethernet controller
                        ethernet_init(device);
                        break;
                }
        }
    }
}
