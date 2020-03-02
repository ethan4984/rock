#pragma once

#include <memory.h>
#include <shitio.h>

void command_handler(const char *input);

void clr_keyboard_entry();

namespace shell
{
	struct command
   	{
		char name[10] = {0};
		int arguments_num;
		int assumed_size;
	};
}
