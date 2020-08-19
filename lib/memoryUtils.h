#pragma once 

#include <stdint.h>

#define ROUNDUP(a, b) (((a) + ((b) - 1)) / (b))

namespace kernel {

template <typename F, typename ...args> using function = F(*)(args...);

void memset(void *src, int64_t data, uint64_t count);

void memset8(uint8_t *src, uint8_t data, uint64_t count);

void memset16(uint16_t *src, uint16_t data, uint64_t count);

void memset32(uint32_t *src, uint32_t data, uint64_t count);

void memset64(uint64_t *src, uint64_t data, uint64_t count);

void memcpy8(uint8_t *dest, uint8_t *src, uint64_t count);

void memcpy16(uint16_t *dest, uint16_t *src, uint64_t count);

void memcpy32(uint32_t *dest, uint32_t *src, uint64_t count);

void memcpy64(uint64_t *dest, uint64_t *src, uint64_t count);

void set(uint8_t *bitmap, uint64_t location);

void clear(uint8_t *bitmap, uint64_t location);

bool isset(uint8_t *bitmap, uint64_t location);

}
