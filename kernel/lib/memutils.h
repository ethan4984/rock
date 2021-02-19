#ifndef MEMUTILS_H_
#define MEMUTILS_H_

#include <cpu.h>

#define DIV_ROUNDUP(a, b) (((a) + ((b) - 1)) / (b))

#define BM_SET(bitmap, location) (bitmap)[(location) / 8] = (bitmap)[(location) / 8] | (1 << ((location) % 8))
#define BM_CLEAR(bitmap, location) (bitmap)[(location) / 8] = (bitmap)[(location) / 8] & (~(1 << ((location) % 8)))
#define BM_TEST(bitmap, location) ((bitmap[(location) / 8] >> ((location) % 8)) & 0x1)

#define ALIGN_UP(x, a) ({ \
    typeof(x) value = x; \
    typeof(a) align = a; \
    value = DIV_ROUNDUP(value, align) * align; \
    value; \
})

#define ABS(n1, n2) (n1) > (n2) ? n1 - n2 : n2 - n1

typedef void *symbol[];

void memset8(uint8_t *src, uint8_t data, size_t count);
void memset16(uint16_t *src, uint16_t data, size_t count);
void memset32(uint32_t *src, uint32_t data, size_t count);
void memset64(uint64_t *src, uint64_t data, size_t count);

void memcpy8(uint8_t *dest, uint8_t *src, size_t count);
void memcpy16(uint16_t *dest, uint16_t *src, size_t count);
void memcpy32(uint32_t *dest, uint32_t *src, size_t count);
void memcpy64(uint64_t *dest, uint64_t *src, size_t count);

size_t strlen(const char *str);
int strcmp(const char *str0, const char *str1);
int strncmp(const char *str0, const char *str1, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strchr(const char *str, int c);
char *strtok_r(char *__restrict s, const char *__restrict del, char **__restrict m);
char *strtok(char *__restrict s, const char *__restrict delimiter);
char *str_congregate(char *str1, char *str2);
size_t last_char(char *str, char c);

char *itob(size_t num, size_t base);
int atoi(const char *str);

void bm_alloc_region(uint8_t *bitmap, size_t start, size_t limit); 
void bm_free_region(uint8_t *bitmap, size_t start, size_t limit);
int bm_first_free(uint8_t *bitmap, size_t size);

#endif
