#include <stdint.h>

#include "paging.h"
#include "shitio.h"

using namespace standardout;

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
		k_print("VIRTUAL MAP: %x to %x\n", virtual_addr, physical_addr);
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

		static bool was_done = false;
		if(!was_done) {
			new_virtual_map(0, 0);
			new_virtual_map(0x400000, 0x400000);
			enable_page();
			was_done = true;
		}
	}

	void *virtual_address_space::get_physical_address(void *virtual_addr) {
		uint32_t pdindex = (uint32_t)virtual_addr >> 22;
		uint32_t ptindex = (uint32_t)virtual_addr >> 12 & 0x03FF;

		uint32_t *pd = (uint32_t*)0xFFFFF000;

		uint32_t *pt = ((uint32_t*)0xFFC00000) + (0x400 * pdindex);

		return (void*)((pt[ptindex] & ~ 0xFFF) + ((uint32_t)virtual_addr & 0xFFF));
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






