#include "port.h"
#include "shitio.h"
#include "keyboard.h"
#include "memory.h"
#include "shell.h"

char keyboard_map[] = {
			' ', ' ', '1', '2', '3',  '4', '5', '6',  '7', '8', '9', '0',
                   	'-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
                   	'o', 'p', '[', ']', ' ',  ' ', 'a', 's',  'd', 'f', 'g', 'h',
                   	'j', 'k', 'l', ';', '\'', '`', ' ', '\\', 'z', 'x', 'c', 'v',
                   	'b', 'n', 'm', ',', '.',  '/', ' ', ' ',  ' ', ' '
		      };

struct keyboard_buffer {
	char *input;
	bool takingInput;
} key_entry;

void clr_keyboard_entry() {
	memset(key_entry.input, 0, strlen(key_entry.input));
}

char *get_entry() {
	return key_entry.input;
}

void startInput() {
	key_entry.takingInput = true;
}

extern "C" void keyboard_handler_main() {
	outb(0x20, 0x20);

	static int counter = 0;

	if(inb(0x64) & 0x01) {

		char keycode = inb(0x60);

		if(keycode < 0)
			return;

		switch(keycode) {
			case 0x1c:
				if(!key_entry.takingInput) {
					putchar('\n');
					break;
				}

				key_entry.takingInput = true;

				command_handler(key_entry.input);

				counter = 0;

				break;
			case 0x0e:
				if(!key_entry.takingInput) { // minor buffering issues - fix me
					putchar('\b');
					break;
				}

				putchar('\b');

				key_entry.input[counter] = 0;

				counter -= 1;
				break;
			default:
				if(!key_entry.takingInput) {
					putchar(keyboard_map[(unsigned char) keycode]);
					break;
				}
				putchar(keyboard_map[(unsigned char) keycode]);
				key_entry.input[counter++] = keyboard_map[(unsigned char)keycode];
				break;
		}
	}
	else
		t_print("IRQ1: bad keycodes");
}
