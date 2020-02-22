#pragma once

#define COM1 0x3f8 //serial ports
#define COM2 0x2f8
#define COM3 0x3e8
#define COM4 0x2e8

#include <stdint.h>

void serial_init();

inline uint8_t serial_read();

void serial_write(uint8_t data);

void outb(uint16_t port, uint8_t data);

void outw(uint16_t port, uint16_t data);

void outl(uint16_t port, uint32_t data);

uint8_t inb(uint16_t port);

uint16_t inw(uint16_t port);

uint32_t inl(uint16_t port);

inline void io_wait(void);

inline bool are_interrupts_enabled();
