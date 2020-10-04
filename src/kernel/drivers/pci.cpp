#include <kernel/mm/kHeap.h>
#include <kernel/drivers/pci.h>
#include <lib/asmUtils.h>
#include <lib/output.h>

namespace pci {

uint32_t pciRead(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
    outd(0xcf8, (1 << 31) | ((uint32_t)bus << 16) | (((uint32_t)device & 31) << 11) | (((uint32_t)func & 7) << 8) | ((uint32_t)reg & ~(3)));
    return ind(0xcfc);
}

void pciWrite(uint32_t data, uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
    outd(0xcf8, (1 << 31) | ((uint32_t)bus << 16) | (((uint32_t)device & 31) << 11) | (((uint32_t)func & 7) << 8) | ((uint32_t)reg & ~(3)));
    outd(0xcfc, data);
}

void checkDevice(uint8_t bus, uint8_t device, uint8_t function) {
    bool isBridge = true;

    if(((uint8_t)(pciRead(bus, device, function, 0xC) >> 16) & ~(1 << 7)) != 1 || (uint8_t)(pciRead(bus, device, function, 0x8) >> 24) != 6 || (uint8_t)(pciRead(bus, device, function, 0x8) >> 16) != 4)
        isBridge = false;

    if(isBridge) {
        pciScanBus((uint8_t)(pciRead(bus, device, function, 0x18) >> 8)); /* gets the seconadary bus */ 
    } else {
        pciInfo_t newDevice = { (uint8_t)(pciRead(bus, device, function, 0x8) >> 24), // class
                            (uint8_t)(pciRead(bus, device, function, 0x8) >> 16), // subclass
                            (uint8_t)(pciRead(bus, device, function, 0x8) >> 8), // progIF
                            (uint16_t)(pciRead(bus, device, function, 0) >> 16), // deviceID
                            (uint16_t)pciRead(bus, device, function, 0), // vendorID
                            bus,
                            device,
                            function
                          };

        addPCIDevice(newDevice);
    }
}

void pciScanBus(uint8_t bus) {
    for(uint8_t device = 0; device < 32; device++) {
        if((uint16_t)pciRead(bus, device, 0, 0) != 0xffff) {
            checkDevice(bus, device, 0);
            
            if((uint8_t)(pciRead(bus, device, 0, 0xc) >> 16) & (1 << 7)) {
                for(uint8_t function = 1; function < 8; function++) {
                    if((uint16_t)pciRead(bus, device, function, 0) != 0xffff)
                        checkDevice(bus, device, function);
                }
            }
        }
    }
}

pciBar_t getBAR(pciInfo_t device, uint64_t barNum) {
    uint32_t base = pciRead(device.bus, device.device, device.function, 0x10 + (barNum * 4));

    pciWrite(0xffffffff, device.bus, device.device, device.function, 0x10 + (barNum * 4));
    uint32_t size = pciRead(device.bus, device.device, device.function, 0x10 + (barNum * 4));
    pciWrite(base, device.bus, device.device, device.function, 0x10 + (barNum * 4));
    
    return (pciBar_t) { base, ~(size) + 1 };
}

void addPCIDevice(pciInfo_t newDevice) {
    static uint64_t maxSize = 10;

    if(pciDevices.totalDevices > maxSize) {
        maxSize += 10;
        pciDevices.devices = (pciInfo_t*)kheap.krealloc(pciDevices.devices, maxSize);
    }

    pciDevices.devices[pciDevices.totalDevices++] = newDevice;
}

void init() {
    pciDevices.devices = new pciInfo_t[10];

    pciScanBus(0);
    showDevices();
}

void showDevices() { 
    for(uint64_t device = 0; device < pciDevices.totalDevices; device++) {
        kprintDS("[PCI]", "device: %x", device);
        kprintDS("[PCI]","\tVendor: %x on [bus] %x [function] %x", pciDevices.devices[device].vendorID, pciDevices.devices[device].bus, pciDevices.devices[device].function);
        kprintDS("[PCI]", "\tDevice type: [class] %x [subclass] %x [progIF] %x [Device ID] %x", pciDevices.devices[device].classCode, pciDevices.devices[device].subclass, pciDevices.devices[device].progIF, pciDevices.devices[device].deviceID);
    }
    kprintDS("[PCI]", "Total Devices: %x", pciDevices.totalDevices);
}

}
