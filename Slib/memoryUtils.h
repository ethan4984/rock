#pragma once

#include <stdint.h>

#define KERNEL_HIGH_VMA 0xFFFFFFFF80000000
#define HIGH_VMA 0xFFFF800000000000

#define TABLESIZE 0x1000
#define PAGESIZE 0x1000

#define RMFLAGS 0x000FFFFFFFFFF000
#define GETFLAGS ~RMFLAGS
#define TABLEPRESENT (1 << 0)
#define TABLEWRITE (1 << 1)
#define TABLEUSER (1 << 2)
#define TABLEHUGE (1 << 7)

#define SUPERVISOR 0
#define USER 1

void memset(void *src, int64_t data, uint64_t count);

void memset8(uint8_t *src, uint8_t data, uint64_t count);

void memset16(uint16_t *src, uint16_t data, uint64_t count);

void memset32(uint32_t *src, uint32_t data, uint64_t count);

void memset64(uint64_t *src, uint64_t data, uint64_t count);

void memcpy(void *src, const void *tar, int how_many);

void memcpy64(uint64_t *dest, const uint64_t *src, uint64_t n);

void memmove(uint64_t *dest, const uint64_t *src, uint64_t n);

void set(uint8_t *bitmap, uint64_t location);

void clear(uint8_t *bitmap, uint64_t location);

bool isset(uint8_t *bitmap, uint64_t location);

uint64_t *getPageDirectory();

void setPageDirectory();

void invlpg(uint64_t* vaddr);
