#ifndef PCI_H_
#define PCI_H_

#include <cpu.h>

struct pci_device {
    uint8_t class_code;
    uint8_t sub_class;
    uint8_t prog_IF;
    uint16_t device_ID;
    uint16_t vendor_ID;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
};

struct pci_bar {
    uint32_t base;
    uint32_t size;
};

extern struct pci_device *pci_devices;
extern uint64_t pci_device_cnt;

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg);

void pci_write(uint32_t data, uint8_t bus, uint8_t device, uint8_t func, uint8_t reg);

void pci_scan_bus(uint8_t bus);

struct pci_bar pci_get_bar(struct pci_device device, uint64_t bar_num);

void pci_init();

void show_pci_devices();

#endif
