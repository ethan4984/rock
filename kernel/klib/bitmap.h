#ifndef __MM_BITMAP_H_
#define __MM_BITMAP_H_

#define BLOCK_SIZE 32

#include <asmutils.h>
#include <memutils.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

void bitmap_init();

void *kmalloc(uint64_t cnt); 

void *kcalloc(uint64_t cnt); 

uint64_t kfree(void *addr);

void *krealloc(void *addr, uint64_t size);

#endif
