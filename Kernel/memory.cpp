#include "shitio.h"
#include "memory.h"

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

void memcpy(void *src, void *tar, int how_many) {
	char *Rsrc = (char*)src;
	char *Rtar = (char*)tar;

	for(int i = 0; i < how_many; i++)
		Rsrc[i] = Rtar[i];
}
