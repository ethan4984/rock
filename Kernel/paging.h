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
	class block : protected virtual_address_space {
		public:
			void *malloc(size_t size);

			void free(void *location);
		private:
			uint32_t *used;

			uint32_t *is_used; //1 if full 0 if not 1
	};
}

void setup();

void current_address_spaces();

void block_map();
