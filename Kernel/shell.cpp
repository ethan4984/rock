#include "shell.h"
#include "shitio.h"

void version() {
	k_print("\ncrepOS beta 1.1\n");
}

char *command_list[] = { "version" };

typedef void (*command_functions)();
command_functions comm_func[] = { version };

void command_handler(char *input) {
	for(int i = 0; i < sizeof command_list/sizeof *command_list; i++) {
		t_print(input);
		t_print(command_list[i]);
		if(input == command_list[i]) {
			t_print(input);
			t_print(command_list[i]);
			comm_func[i]();
		}
	}
}
