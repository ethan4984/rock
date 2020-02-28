#pragma once

#include <stddef.h>

namespace MM {
	class virtual_address_space {
		public:
			void new_virtual_map(uint32_t physical_addr, uint32_t virtual_addr);

			bool new_claim(uint32_t physical_addr, uint32_t virtual_addr);

			void *get_physical_address(void *virtual_addr);

			void paging_init();

			uint32_t *physical_log;

			uint32_t *virtual_log;

			uint32_t size;
		protected:
			void enable_page();

			uint32_t *page_dir = 0;

			uint32_t page_loc = 0;

			uint32_t *last_page = 0;
	};

	void set(uint32_t location);

	void clear(uint32_t location);

	uint8_t isset(uint32_t location);

	void *malloc(size_t size);

	void free(void *location);

	void page_frame_init(uint32_t mem_range);

	uint32_t allocate_block();

	void free_block(uint32_t block_num);

	uint32_t first_free();
}

void setup();

void current_address_spaces();

void block_show();
