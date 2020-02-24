#include <stdint.h>

#include "paging.h"
#include "shitio.h"

using namespace standardout;

void enable_page();

void new_virtual_map(uint32_t physical_addr, uint32_t virtual_addr);

void new_claim(uint32_t physical_addr, uint32_t virtual_addr);

void get_physical_adddress(void *virtual_addr);

static uint32_t* page_dir = 0;
static uint32_t page_loc = 0;
static uint32_t* last_page = 0;

struct blocks {
	blocks *last;
	blocks *next;

	uint32_t size = 0;
	bool is_full;
} *block;

struct segments {
	uint32_t virtual_space = -69;
	uint32_t physical_space = -69;

	segments *next;

	bool is_full = false;
	uint32_t size = 0;
} *root;

void new_claim(uint32_t physical_addr, uint32_t virtual_addr) {
	root->virtual_space = virtual_addr;
	root->physical_space = physical_addr;

	root->size += 1;
	root->next->size = root->size;
	root = root->next;
}

void paging_init() {
	page_dir = (uint32_t*)0x400000;
	page_loc = (uint32_t)page_dir;
	last_page = (uint32_t*)0x404000;

	for(int i = 0; i < 1024; i++)
		page_dir[i] = 0 | 2;

	new_virtual_map(0, 0);
	new_virtual_map(0x400000, 0x400000);

	enable_page();
}

void enable_page() {
	asm volatile("mov %%eax, %%cr3": :"a"(page_loc));
	asm volatile("mov %cr0, %eax");
	asm volatile("orl $0x80000000, %eax");
	asm volatile("mov %eax, %cr0");
}

void new_virtual_map(uint32_t physical_addr, uint32_t virtual_addr) {
	new_claim(physical_addr, virtual_addr);

	for(int i = 0; i < 1024; i++) {
		last_page[i] = physical_addr | 3;
		physical_addr += 4096;
	}

	page_dir[virtual_addr >> 22] = ((uint32_t)last_page) | 3;
	last_page = (uint32_t *)(((uint32_t)last_page) + 4096);
	k_print("VIRTUAL MAP: %x to %x\n", virtual_addr, physical_addr);
}

void *get_physical_address(void *virtual_addr) {
	uint32_t pdindex = (uint32_t)virtual_addr >> 22;
	uint32_t ptindex = (uint32_t)virtual_addr >> 12 & 0x03FF;

	uint32_t *pd = (uint32_t*)0xFFFFF000;

    	uint32_t *pt = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);

	return (void*)((pt[ptindex] & ~0xFFF) + ((uint32_t)virtual_addr & 0xFFF));
}
