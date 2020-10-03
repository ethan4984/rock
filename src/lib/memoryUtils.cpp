#include <lib/memoryUtils.h>

void memset(void *src, int64_t data, uint64_t count) {
    asm volatile("rep stosb" : "=D"(src),"=c"(count) : "0"(src), "a"(data), "1"(count) : "memory");
}

void memset8(uint8_t *src, uint8_t data, uint64_t count) {
    for(uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memset16(uint16_t *src, uint16_t data, uint64_t count) {
    for(uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memset32(uint32_t *src, uint32_t data, uint64_t count) {
    for (uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memset64(uint64_t *src, uint64_t data, uint64_t count) {
    for (uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memcpy8(uint8_t *dest, uint8_t *src, uint64_t count) {
    for(uint64_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void memcpy16(uint16_t *dest, uint16_t *src, uint64_t count) {
    for(uint64_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void memcpy32(uint32_t *dest, uint32_t *src, uint64_t count) {
    for(uint64_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void memcpy64(uint64_t *dest, uint64_t *src, uint64_t count) {
    for(uint64_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

void set(uint8_t *bitmap, uint64_t location) {
    bitmap[location / 8] = bitmap[location / 8] | (1 << (location % 8));
}

void clear(uint8_t *bitmap, uint64_t location) {
    bitmap[location / 8] = bitmap[location / 8] & (~(1 << (location % 8)));
} 

bool isset(uint8_t *bitmap, uint64_t location) {
    return (bitmap[location / 8] >> (location % 8)) & 0x1;
}
