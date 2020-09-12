#include <kernel/mm/kHeap.h>
#include <kernel/drivers/pci.h>
#include <lib/asmUtils.h>
#include <lib/output.h>

namespace kernel {

uint32_t pci_t::pciRead(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
    outd(0xcf8, (1 << 31) | ((uint32_t)bus << 16) | (((uint32_t)device & 31) << 11) | (((uint32_t)func & 7) << 8) | ((uint32_t)reg & ~(3)));
    return ind(0xcfc);
}

void pci_t::pciWrite(uint32_t data, uint8_t bus, uint8_t device, uint8_t func, uint8_t reg) {
    outd(0xcf8, (1 << 31) | ((uint32_t)bus << 16) | (((uint32_t)device & 31) << 11) | (((uint32_t)func & 7) << 8) | ((uint32_t)reg & ~(3)));
    outd(0xcfc, data);
}

void pci_t::checkDevice(uint8_t bus, uint8_t device, uint8_t function) {
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

void pci_t::pciScanBus(uint8_t bus) {
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

pciBar_t pci_t::getBAR(pciInfo_t device, uint64_t barNum) {
    uint32_t base = pciRead(device.bus, device.device, device.function, 0x10 + (barNum * 4));

    pciWrite(0xffffffff, device.bus, device.device, device.function, 0x10 + (barNum * 4));
    uint32_t size = pciRead(device.bus, device.device, device.function, 0x10 + (barNum * 4));
    pciWrite(base, device.bus, device.device, device.function, 0x10 + (barNum * 4));
    
    return (pciBar_t) { base, ~(size) + 1 };
}

void pci_t::addPCIDevice(pciInfo_t newDevice) {
    static uint64_t maxSize = 10;

    if(totalDevices > maxSize) {
        maxSize += 10;
        pciDevice = (pciInfo_t*)kheap.krealloc(pciDevice, maxSize);
    }

    pciDevice[totalDevices++] = newDevice;
}

void pci_t::initPCI() {
    pciDevice = new pciInfo_t[10];

    pciScanBus(0);
    showDevices();
}

void pci_t::showDevices() { 
    for(uint64_t device = 0; device < totalDevices; device++) {
        cout + "[PCI]" << "device " << device << ":\n";
        cout + "[PCI]" << "\tVendor: " << pciDevice[device].vendorID << " on [bus] " << pciDevice[device].bus << " [function] " << pciDevice[device].function << "\n";

        cout + "[PCI]" << "\tDevice type: [class] " << pciDevice[device].classCode << " [subclass] " << pciDevice[device].subclass << " [progIF] " << pciDevice[device].progIF << " [Device ID] " << pciDevice[device].deviceID << "\n";
    }
    cout + "[PCI]" << "Total Devices: " << totalDevices << "\n";
}

}
