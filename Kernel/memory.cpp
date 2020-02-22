#pragma once

#include <stdint.h>
#include <stddef.h>

void *memset(void *src, int val, size_t how_many) {
    void *new_mem = src;
    asm volatile (	"rep stosb"
        		:"=D"(src),"=c"(how_many)
       			:"0"(src),"a"(val),"1"(how_many)
        		:"memory"
    );
    return new_mem;
}

//toDo: add memcpy, malloc, free, etc
