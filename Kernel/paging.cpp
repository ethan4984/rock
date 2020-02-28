#include <stdint.h>

#include "paging.h"
#include "shitio.h"
#include "memory.h"

using namespace standardout;

extern uint32_t kernel_end;

uint8_t *bitmap = (uint8_t*)(&kernel_end);
uint8_t *mem_start;
uint32_t total_blocks;
uint32_t size;

namespace MM {
	bool virtual_address_space::new_claim(uint32_t physical_addr, uint32_t virtual_addr) {
		for(uint32_t i = 0; i < size; i++) {
			if(physical_log[i] == physical_addr || virtual_log[i] == virtual_addr) {
				t_print("BRUH: stop it, get help (your trying to duplicate an already exist address space");
				return false;
			}
		}
		physical_log[size] = physical_addr;
		virtual_log[size] = virtual_addr;
		size++;

		return true;
	}

	void virtual_address_space::new_virtual_map(uint32_t physical_addr, uint32_t virtual_addr) {
		if(!new_claim(physical_addr, virtual_addr))
			return;

		for(int i = 0; i < 1024; i++) {
			last_page[i] = physical_addr | 3;
			physical_addr += 4096;
		}

		page_dir[virtual_addr >> 22] = ((uint32_t)last_page) | 3;
		last_page = (uint32_t*)(((uint32_t)last_page) + 4096);
	}

	void virtual_address_space::enable_page() {
		asm volatile("mov %%eax, %%cr3": : "a"(page_loc));
		asm volatile("mov %cr0, %eax");
		asm volatile("orl $0x80000000, %eax");
		asm volatile("mov %eax, %cr0");
	}

	void virtual_address_space::paging_init() {
		page_dir = (uint32_t*)0x400000;
		page_loc = (uint32_t)page_dir;
		last_page = (uint32_t*)0x404000;

		for(int i = 0; i < 1024; i++)
			page_dir[i] = 0 | 2;

		new_virtual_map(0, 0);
		new_virtual_map(0x400000, 0x400000);
		enable_page();
		k_print("PAGING: V to P maped between 0x0 and 0x400000\n");
	}

	void set(uint32_t location) {
		bitmap[location / 8] = bitmap[location / 8] | (1 << (location % 8));
	}

	void clear(uint32_t location) {
		bitmap[location / 8] = bitmap[location / 8] & (~(1 << (location % 8)));
	}

	uint8_t isset(uint32_t location) {
		return (bitmap[location / 8] >> (location % 8)) & 0x1;
	}

	void page_frame_init(uint32_t mem_range) {
		total_blocks = mem_range / 0x1000; //4Kb blocks

		size = total_blocks / 8;
		if(size * 8 < total_blocks)
			size++;

	    	memset(bitmap, 0, size);
	    	mem_start = (uint8_t*)((((uint32_t)(bitmap + size)) & 0xfffff000) + 0x1000);

		k_print("PMM: mem range: %d MB\n", mem_range / (1024 * 1024));
		k_print("PMM: blocks num: %d\n", total_blocks);
		k_print("PMM: bitmap addr: %x\n", (uint32_t)bitmap);
		k_print("PMM: bitmap size: %d\n", size);
		k_print("PMM: addr strat: %x\n", (uint32_t)mem_start);
	}

	uint32_t allocate_block() {
		uint32_t free_block = first_free();
		set(free_block);
		return free_block;
	}

	void free_block(uint32_t block_num) {
		clear(block_num);
	}

	uint32_t first_free() {
		for(uint32_t i = 0; i < total_blocks; i++) {
			if(!isset(i))
				return i;
		}
	    	t_print("Bruh: we are running out a blocks, make some more bitch\n");
	    	return (uint32_t) - 1;
	}
}

MM::virtual_address_space obj;

void setup() {
	obj.paging_init();
}

void current_address_spaces() {
	k_print("\nVirtual\tPhysical\n");
	for(uint32_t i = 0; i < obj.size; i++) {
		k_print("%x %x", obj.physical_log[i], obj.virtual_log[i]);
		if(i != obj.size - 1)
			putchar('\n');
	}
}

void block_show() {
	k_print("\nPMM: blocks num: %d\n", total_blocks);
	k_print("PMM: bitmap addr: %x\n", (uint32_t)bitmap);
	k_print("PMM: bitmap size: %d\n", size);
	k_print("PMM: addr strat: %x", (uint32_t)mem_start);
}







