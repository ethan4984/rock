#ifndef __MM_BITMAP_H_
#define __MM_BITMAP_H_

#define BLOCK_SIZE 32

#include <asmutils.h>
#include <memutils.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

void bitmap_init();

void *kmalloc(size_t cnt); 
void *kcalloc(size_t cnt); 
size_t kfree(void *addr);
void *krealloc(void *addr, size_t size);
void *krecalloc(void *addr, size_t size);

#endif
