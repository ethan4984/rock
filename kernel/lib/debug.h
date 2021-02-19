#ifndef DEBUG_H_
#define DEBUG_H_

#include <memutils.h>
#include <stdarg.h>

uint8_t serial_read();
void serial_write(uint8_t data);
void serial_write_str(const char *str);

void print_args(const char *str, va_list arg, void (*fp)(uint8_t));
void kprintf(const char *str, ...);
void sprintf(char *buffer, const char *str, int null_term, ...);

#endif
