#ifndef RSDP_H_
#define RSDP_H_

#include <acpi/tables.h>
#include <mm/vmm.h>

void rsdp_init();

void *find_SDT(const char *signature);

#endif
