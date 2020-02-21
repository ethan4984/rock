#include "port.h"
#include "shitio.h"
#include "keyboard.h"
#include "memory.h"
#include "shell.h"

unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
  '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
 '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',
  'm', ',', '.', '/',   0,
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

struct keyboard_buffer {
	char *input;
	bool takingInput;
} key_entry;

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

				memset(key_entry.input, 0, strlen(key_entry.input));
				counter = 0;

				break;
			case 0x0e:
				if(!key_entry.takingInput) { //kb buffer issues when we do backspaces - fix me
					putchar('\b');
					break;
				}

				key_entry.input[strlen(key_entry.input)] = 0;
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
