#pragma once

#include <stdint.h>
#include <stddef.h>

class pci_device_t
{
    public:
        pci_device_t(uint8_t vaild_t, uint8_t bus_t, uint8_t device_t, uint8_t function_t);

        uint8_t vaild;
        uint8_t bus;
        uint8_t device;
        uint8_t function;
};

class pci_device_id_t
{
    public:
        pci_device_id_t(uint8_t bus, uint8_t device, uint8_t function);

        uint8_t class_code;
        uint8_t subclass;
        uint8_t prog_if;
        uint16_t device_id;
        uint16_t vendor_id;
        uint8_t vaild;
        uint8_t bus;
        uint8_t device;
        uint8_t function;
};

extern pci_device_t *pci_devices;
extern pci_device_id_t *pci_device_ids;
extern uint64_t total_devices;

uint32_t pci_read(uint8_t bus_t, uint8_t device_t, uint8_t func_t, uint8_t reg_t);

void pci_write(uint32_t data, uint8_t bus_t, uint8_t device_t, uint8_t func_t, uint8_t reg_t);

uint8_t pci_is_multifunction(uint8_t bus, uint8_t device);

uint16_t pci_get_vendor(uint8_t bus, uint8_t device, uint8_t func);

void pci_scan(uint8_t bus);

void check_device(uint8_t bus, uint8_t device, uint8_t function);

void add_pci_device(pci_device_t new_device);

uint8_t pci_multifunction(uint8_t bus, uint8_t device);

uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t func);

uint8_t read_secondary_bus(uint8_t bus, uint8_t device, uint8_t function);

void add_device(pci_device_t new_device, pci_device_id_t new_device_id);

bool is_bridge(uint8_t bus, uint8_t device, uint8_t function);

uint32_t pci_get_mmio_bar(pci_device_id_t device, uint8_t bar);
uint32_t pci_get_mmio_bar_size(pci_device_id_t device, uint8_t bar);

void pci_init();


struct pci_bar_t {
    uintptr_t base;
    size_t size;

    int is_mmio;
    int is_prefetchable;
};

int pci_read_bar(pci_device_id_t device, int bar, pci_bar_t *out);
