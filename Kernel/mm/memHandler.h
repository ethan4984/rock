#pragma once

#include <Kernel/stivale.h>

#include <stdint.h>

class allocationIndexs_t {
	public:
		void *address;
		uint64_t sizeOfAllocation;
} __attribute__((packed));

void memInit(stivaleInfo_t *bootInfo);

void *malloc(uint64_t size);

void *realloc(void *oldAddress, uint64_t size);

void free(void *address);
