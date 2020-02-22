#include <stdint.h>

/* not working : we get a triple fault */

uint32_t page_dir[1024] __attribute__((aligned(4096)));
uint32_t page_table[1024] __attribute__((aligned(4096)));

extern void startPaging() asm("start_paging");
extern void loadPaging(long unsigned int*) asm("load_paging");

void page_dir_init() {
	for(int i = 0; i < 1024; i++)
    		page_dir[i] = 0x00000002;
}

void page_init() {

	page_dir_init();

	for(int i = 0; i < 1024; i++)
		page_table[i] = (i * 0x1000) | 3;

	page_dir[0] = ((unsigned int)page_table) | 3;

	loadPaging(page_dir);
	startPaging();
}
