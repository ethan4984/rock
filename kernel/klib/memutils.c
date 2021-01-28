#include <memutils.h>
#include <bitmap.h>

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
