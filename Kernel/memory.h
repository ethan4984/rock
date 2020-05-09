#pragma once

#include <stdint.h>

void memset32(uint32_t *dst, uint32_t data, uint64_t how_many);

void memset64(uint64_t *dst, uint64_t data, uint64_t how_many);

void *memset(void *src, int val, unsigned int how_many);

void memcpy(void *src, const void *tar, int how_many);

void grab_reg();
