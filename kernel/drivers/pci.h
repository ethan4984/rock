#pragma once

#include <stdint.h>

namespace kernel {

struct pciInfo_t {
    uint8_t classCode;
    uint8_t subclass;
    uint8_t progIF;
    uint16_t deviceID;
    uint16_t vendorID;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
};

struct pciBar_t {
    uint32_t base;
    uint32_t size;
};

class pci_t {
public:
    void initPCI();

    void showDevices();

    uint32_t pciRead(uint8_t bus, uint8_t device, uint8_t func, uint8_t reg);

    void pciWrite(uint32_t data, uint8_t bus, uint8_t device, uint8_t func, uint8_t reg);

    void pciScanBus(uint8_t bus);

    void checkDevice(uint8_t bus, uint8_t device, uint8_t function);

    pciBar_t getBAR(pciInfo_t device, uint64_t barNum);

    pciInfo_t *pciDevice;

    uint64_t totalDevices;
private:
    void addPCIDevice(pciInfo_t newDevice);
};

inline pci_t pci;

}
