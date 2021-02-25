#ifndef PCI_H_
#define PCI_H_

#include <cpu.h>
#include <vec.h>

struct pci_device {
    uint8_t class_code;
    uint8_t sub_class;
    uint8_t prog_if;
    uint16_t device_id;
    uint16_t vendor_id;
    uint8_t bus;
    uint8_t device;
    uint8_t func;
};

struct pci_bar {
    uint32_t base;
    uint32_t size;
};

extern_vec(struct pci_device, pci_device_list);

uint32_t pci_read(struct pci_device *device, uint8_t off);
void pci_write(struct pci_device *device, uint8_t off, uint32_t data);
void pci_become_bus_master(struct pci_device *device);
void pci_set_msi(struct pci_device *device, uint8_t vec);
void pci_scan_bus(uint8_t bus);
void pci_scan_device(uint8_t bus, uint8_t device, uint8_t func);
int pci_get_bar(struct pci_device *device, struct pci_bar *ret, size_t bar_num);
void pci_scan();

#endif
