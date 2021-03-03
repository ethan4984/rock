#ifndef MMAP_H_
#define MMAP_H_

#include <mm/vmm.h>

#define MAP_FAILED (void*)-1
#define MAP_PRIVATE 0x1
#define MAP_SHARED 0x2
#define MAP_FIXED 0x4
#define MAP_ANONYMOUS 0x8

#define PROT_NONE  0x00
#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define PROT_EXEC  0x04

#define MMAP_MIN_ADDR 0x10000

void *mmap(struct page_map *page_map, void *addr, size_t length, int prot, int flags, int fd, int64_t off);
int munmap(struct page_map *page_map, void *addr, size_t length);
void mmap_reserve(struct page_map *page_map, void *addr, size_t length);

#endif
