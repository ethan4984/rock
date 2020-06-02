#include <Kernel/pci.h> 
#include <Kernel/mm/memHandler.h>
#include <Slib/ports.h>
#include <Slib/videoIO.h>

using namespace out;

pciDevice *pciDevices;
uint64_t totalDevices = 0;

pciDevice::pciDevice(uint8_t bus_t, uint8_t device_t, uint8_t func_t) : bus(bus_t), device(device_t), function(func_t)
{
    classCode = (uint8_t)(pciRead(bus_t, device_t, func_t, 0x8) >> 24);
    subclass = (uint8_t)(pciRead(bus_t, device_t, func_t, 0x8) >> 16);
    progIF = (uint8_t)(pciRead(bus_t, device_t, func_t, 0x8) >> 8);
    deviceID = (uint16_t)(pciRead(bus_t, device_t, func_t, 0) >> 16);
    vendorID = (uint16_t)pciRead(bus_t, device_t, func_t, 0);
}

void addDevice(pciDevice newDevice) 
{
    static uint64_t maxDevices = 10;
    if(totalDevices == maxDevices) {
        realloc(pciDevices, sizeof(pciDevices) * 10);
        maxDevices += 10;
    }
    pciDevices[totalDevices++] = newDevice;
}

uint32_t pciRead(uint8_t bus_t, uint8_t device_t, uint8_t func_t, uint8_t reg_t)
{
    uint32_t bus = (uint32_t)bus_t, device = (uint32_t)device_t, func = (uint32_t)func_t;
    uint32_t address = (1 << 31) | (bus << 16) | ((device & 31) << 11) | ((func & 7) << 8) | (reg_t & ~(3));
    outd(0xCF8, address);
    return ind(0xCFC);
}

void pci_write(uint32_t data, uint8_t bus_t, uint8_t device_t, uint8_t func_t, uint8_t reg_t)
{
    uint32_t bus = (uint32_t)bus_t, device = (uint32_t)device_t, func = (uint32_t)func_t;
    uint32_t address = (1 << 31) | (bus << 16) | ((device & 31) << 11) | ((func & 7) << 8) | (reg_t & ~(3));
    outd(0xCF8, address);
    outd(0xCFC, data);
}

void checkDevice(uint8_t bus, uint8_t device, uint8_t function)
{
    bool isBridge = true;

    if(((uint8_t)(pciRead(bus, device, function, 0xC) >> 16) & ~(1 << 7)) != 1 || (uint8_t)(pciRead(bus, device, function, 0x8) >> 24) != 6 || (uint8_t)(pciRead(bus, device, function, 0x8) >> 16) != 4)
        isBridge = false;

    if(isBridge) 
        pciScanBus((uint8_t)(pciRead(bus, device, function, 0x18) >> 8)); /* gets the seconadary bus */
    else {
        pciDevice newDevice(bus, device, function);
        addDevice(newDevice);
    }
}

void pciScanBus(uint8_t bus) 
{
    for(uint8_t device = 0; device < 32; device++) { /* loops through all 32 devices */
        if((uint16_t)pciRead(bus, device, 0, 0) != 0xffff) { /* grabs vendor and checks if it is a vaild device 0xFFFF == invaild device */
            checkDevice(bus, device, 0); /* Adds the device if we can */

            if((uint8_t)(pciRead(bus, device, 0, 0xC) >> 16) & (1 << 7)) { /* is this a multifunction device ? */
                for(uint8_t function = 1; function < 8; function++) {
                    if((uint16_t)pciRead(bus, device, function, 0) != 0xffff) /* vaild device? */
                        checkDevice(bus, device, 0);
                }
            }
        }
    }
}

void pciInit()
{
    bool first = true;
    if(first) {
        pciDevices = (pciDevice*)malloc(sizeof(pciDevice) * 10);
        first = false;
    }

    pciScanBus(0);
    for(uint64_t device = 0; device < totalDevices; device++) {
        kPrint("\nPCI Device %d: \n", device);
        kPrint("\tVendor: %x on [bus] %d [device] %d [function] %d\n", pciDevices[device].vendorID, pciDevices[device].bus, pciDevices[device].device, pciDevices[device].function);
        kPrint("\tDevice type: [class] %d [subclass] %d [progIF] %d [Device ID] %x\n", pciDevices[device].classCode, pciDevices[device].subclass, pciDevices[device].progIF, pciDevices[device].deviceID);
    }
    kPrint("\nPCI: Total Devices: %d\n", totalDevices);
}

pciDevice *grabPCIdevices() 
{
    return pciDevices;
}
