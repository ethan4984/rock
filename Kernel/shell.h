#pragma once

#include "memory.h"
#include "shitio.h"

void command_handler(const char *input);

void clr_keyboard_entry();

char *get_entry();

namespace shell {
	struct command {
		char name[10];
		int arguments_num;
		int assumed_size;
	};
}
