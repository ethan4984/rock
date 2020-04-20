#include <shitio.h>
#include <alloc.h>
#include <stdint.h>
#include <port.h>
#include <pci.h>
#include <ahci.h>

using namespace standardout;

pci_device_t::pci_device_t(uint8_t vaild_t, uint8_t bus_t, uint8_t device_t, uint8_t function_t) : vaild(vaild_t), bus(bus_t), device(device_t), function(function_t)
{

}

pci_device_id_t::pci_device_id_t(uint8_t bus_t, uint8_t device_t, uint8_t func_t)
{
    class_code = (uint8_t)(pci_read(bus_t, device_t, func_t, 0x8) >> 24);
    subclass = (uint8_t)(pci_read(bus_t, device_t, func_t, 0x8) >> 16);
    prog_if = (uint8_t)(pci_read(bus_t, device_t, func_t, 0x8) >> 8);
    device_id = (uint16_t)(pci_read(bus_t, device_t, func_t, 0) >> 16);
    vendor_id = pci_get_vendor(bus_t, device_t, func_t);
}

pci_device_t *pci_devices;
pci_device_id_t *pci_device_ids;

uint64_t total_devices;
uint64_t current_position = 0;
uint64_t max_position = 9;

uint32_t pci_read(uint8_t bus_t, uint8_t device_t, uint8_t func_t, uint8_t reg_t)
{
    uint32_t bus = (uint32_t)bus_t, device = (uint32_t)device_t, func = (uint32_t)func_t;
    uint32_t address = (1 << 31) | (bus << 16) | ((device & 31) << 11) | ((func & 7) << 8) | (reg_t & ~(3));
    outl(0xCF8, address);
    return inl(0xCFC);
}

void pci_write(uint32_t data, uint8_t bus_t, uint8_t device_t, uint8_t func_t, uint8_t reg_t)
{
    uint32_t bus = (uint32_t)bus_t, device = (uint32_t)device_t, func = (uint32_t)func_t;
    uint32_t address = (1 << 31) | (bus << 16) | ((device & 31) << 11) | ((func & 7) << 8) | (reg_t & ~(3));
    outl(0xCF8, address);
    outl(0xCFC, data);
}

void check_device(uint8_t bus, uint8_t device, uint8_t function)
{
    if(is_bridge(bus, device, function))
        pci_scan(read_secondary_bus(bus, device, function));
    else {
        pci_device_t new_pci_device(1, bus, device, function);
        pci_device_id_t new_pci_device_id(bus, device, function);
        add_device(new_pci_device, new_pci_device_id);
    }
}

void pci_scan(uint8_t bus)
{
    for(uint8_t device = 0; device < 32; device++) {
        if(pci_get_vendor(bus, device, 0) != 0xFFFF) { // is vaild device
            check_device(bus, device, 0); // Add device

            if(pci_multifunction(bus, device)) { // is multifunction
                for(uint8_t func = 1; func < 8; func++) {
                    if(pci_get_vendor(bus, device, func) != 0xFFFF)
                        check_device(bus, device, func); // Add device
                }
            }
        }
    }
}

void pci_init() {
    k_print("\nPCI init:\n\n");
    total_devices = 0;
    pci_devices = (pci_device_t*)malloc(sizeof(pci_device_t) * 10); // 10 pci devices
    pci_device_ids = (pci_device_id_t*)malloc(sizeof(pci_device_id_t) * 10); // 10 pci devices

    pci_scan(0); // scan bus 0 and all of its derivatives
    for (uint64_t device = 0; device < current_position; device++) {
        pci_device_t device_info = pci_devices[device];
        pci_device_id_t device_ids = pci_device_ids[device];
        k_print("\tPCI Device %d:\n", device);
        k_print("\t\tVendor: %x on [bus] %d [device] %d [function] %d\n", (uint32_t)pci_get_vendor(device_info.bus, device_info.device, device_info.function), (uint32_t)device_info.bus, (uint32_t)device_info.device, (uint32_t)device_info.function);
        k_print("\t\tDevice type: [class] %d [subclas] %d [prog_if] %d [Device ID] %x\n\n", device_ids.class_code, device_ids.subclass, device_ids.prog_if, (uint32_t)device_ids.device_id);
    }
    k_print("PCI: Total Devices: %d\n\n", total_devices);
}

uint8_t pci_is_multifunction(uint8_t bus, uint8_t device)
{
    return ((uint8_t)(pci_read(bus, device, 0, 0xC) >> 16)) & (1 << 7);
}

uint16_t pci_get_vendor(uint8_t bus, uint8_t device, uint8_t func)
{
    return (uint16_t)pci_read(bus, device, func, 0);
}

uint16_t pci_get_vendor_id(uint8_t bus, uint8_t device, uint8_t func)
{
    return (uint16_t)(pci_read(bus, device, func, 0) >> 16);
}

uint8_t read_secondary_bus(uint8_t bus, uint8_t device, uint8_t function)
{
    return (uint8_t)(pci_read(bus, device, function, 0x18) >> 8);
}

uint8_t pci_multifunction(uint8_t bus, uint8_t device) {
    return (uint8_t)(pci_read(bus, device, 0, 0xC) >> 16) & (1 << 7);
}

bool is_bridge(uint8_t bus, uint8_t device, uint8_t function)
{
    if((uint8_t)(pci_read(bus, device, function, 0xC) >> 16) & ~(1 << 7) != 1)
        return false;
    if((uint8_t)(pci_read(bus, device, function, 0x8) >> 24) != 6)
        return false;
    if((uint8_t)(pci_read(bus, device, function, 0x8) >> 16) != 4)
        return false;
    return true;
}

void add_device(pci_device_t new_device, pci_device_id_t new_device_id)
{
    pci_devices[current_position] = new_device;
    pci_device_ids[current_position] = new_device_id;
    if(current_position == max_position) {
        free(pci_devices);
        pci_devices = (pci_device_t*)malloc(sizeof(pci_device_t) * (10 + max_position + 1));
        free(pci_device_ids);
        pci_device_ids = (pci_device_id_t*)malloc(sizeof(pci_device_id_t) * (10 + max_position + 1));
        max_position += 10;
    }
    current_position++;
    total_devices++;
}
