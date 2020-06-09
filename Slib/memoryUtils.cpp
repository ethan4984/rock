#include <memoryUtils.h>

void memset(void *src, int64_t data, uint64_t count)
{
    asm volatile("rep stosb" : "=D"(src),"=c"(count) : "0"(src),"a"(data),"1"(count) : "memory");
}

void memset8(uint8_t *src, uint8_t data, uint64_t count) 
{
    for(uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memset16(uint16_t *src, uint16_t data, uint64_t count) 
{
    for(uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memset32(uint32_t *src, uint32_t data, uint64_t count) 
{
    for (uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memset64(uint64_t *src, uint64_t data, uint64_t count) 
{
    for (uint64_t i = 0; i < count; i++)
        *src++ = data;
}

void memcpy(void *src, const void *tar, int how_many)
{
    char *Rsrc = (char*)src;
    char *Rtar = (char*)tar;

    for(int i = 0; i < how_many; i++)
        Rsrc[i] = Rtar[i];
}

void memcpy64(uint64_t *dest, const uint64_t *src, uint64_t n)
{
    uint64_t *pdest = dest;
    const uint64_t *psrc = src;

    for (uint64_t i = 0; i < (n / sizeof(uint64_t)); i++) {
        pdest[i] = psrc[i];
    }
}

void memmove(uint64_t *dest, const uint64_t *src, uint64_t n) {
    uint64_t *pdest = dest;
    const uint64_t *psrc = src;

    if (src > dest) {
        for (uint64_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (uint64_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }
}

void set(uint8_t *bitmap, uint64_t location)
{
    bitmap[location / 8] = bitmap[location / 8] | (1 << (location % 8));
}

void clear(uint8_t *bitmap, uint64_t location)
{
    bitmap[location / 8] = bitmap[location / 8] & (~(1 << (location % 8)));
} 

bool isset(uint8_t *bitmap, uint64_t location)
{
    return (bitmap[location / 8] >> (location % 8)) & 0x1;
}

uint64_t *getPageDirectory() 
{
    uint64_t *address;
    asm volatile("movq %%cr3, %0" : "=r"(address));
    return address;
}

void setPageDirectory(uint64_t newPD)
{
    asm volatile("movq %0, %%cr3" :: "r"(newPD) : "memory");
}

void invlpg(uint64_t* vaddr) 
{
    asm volatile("invlpg (%0);" ::"r"(vaddr) : "memory");
}
