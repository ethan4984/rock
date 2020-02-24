#pragma once

#include <stddef.h>

void paging_init();

void *malloc(size_t size);

void free(void *location);
