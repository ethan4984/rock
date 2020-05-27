#pragma once 

#include <stdint.h>

class pciDevice { 
    public: 
        pciDevice(uint8_t bus_t, uint8_t device_t, uint8_t func_t);

        uint8_t classCode;
        uint8_t subclass;
        uint8_t progIF;
        uint16_t deviceID;
        uint16_t vendorID;
        uint8_t vaild;
        uint8_t bus;
        uint8_t device;
        uint8_t function;
};

uint32_t pciRead(uint8_t bus_t, uint8_t device_t, uint8_t func_t, uint8_t reg_t);

void pciWrite(uint32_t data, uint8_t bus_t, uint8_t device_t, uint8_t func_t, uint8_t reg_t);

void pciScanBus(uint8_t bus);

void pciInit();

pciDevice *grabPCIdevices();
