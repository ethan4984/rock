#include <stdint.h>
#include <stddef.h>

#include "shitio.h"

using namespace standardout;

void *memset(void *src, int val, unsigned int how_many) {
    void *new_mem = src;
    asm volatile (	"rep stosb"
        		:"=D"(src),"=c"(how_many)
       			:"0"(src),"a"(val),"1"(how_many)
        		:"memory"
    );
    return new_mem;
}


