#ifndef MEMUTILS_H_
#define MEMUTILS_H_

#include <stdint.h>
#include <stddef.h>

#define ROUNDUP(a, b) (((a) + ((b) - 1)) / (b))

#define BM_SET(bitmap, location) (bitmap)[(location) / 8] = (bitmap)[(location) / 8] | (1 << ((location) % 8))
#define BM_CLEAR(bitmap, location) (bitmap)[(location) / 8] = (bitmap)[(location) / 8] & (~(1 << ((location) % 8)));
#define BM_TEST(bitmap, location) ((bitmap[(location) / 8] >> ((location) % 8)) & 0x1)

typedef void *symbol[];

void memset8(uint8_t *src, uint8_t data, uint64_t count);

void memset16(uint16_t *src, uint16_t data, uint64_t count);

void memset32(uint32_t *src, uint32_t data, uint64_t count);

void memset64(uint64_t *src, uint64_t data, uint64_t count);

void memcpy8(uint8_t *dest, uint8_t *src, uint64_t count);

void memcpy16(uint16_t *dest, uint16_t *src, uint64_t count);

void memcpy32(uint32_t *dest, uint32_t *src, uint64_t count);

void memcpy64(uint64_t *dest, uint64_t *src, uint64_t count);

uint64_t strlen(const char *str);

int strcmp(const char *str0, const char *str1);

int strncmp(const char *str0, const char *str1, uint64_t n);

char *strcpy(char *dest, const char *src);

char *strncpy(char *dest, const char *src, uint64_t n);

char *itob(uint64_t num, uint64_t base);

char *strtok(char *str, const char *delim);

char *strtok_r(char *__restrict s, const char *__restrict del, char **__restrict m);

int character_cnt(const char *str, char c);

#endif
