#include <drivers/ps2_keyboard.h>
#include <fs/fd.h>

static char key_map[] = {   ' ', ' ', '1', '2', '3',  '4', '5', '6',  '7', '8', '9', '0',
                            '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
                            'o', 'p', '[', ']', ' ',  ' ', 'a', 's',  'd', 'f', 'g', 'h',
                            'j', 'k', 'l', ';', '\'', '`', ' ', '\\', 'z', 'x', 'c', 'v',
                            'b', 'n', 'm', ',', '.',  '/', ' ', ' ',  ' ', ' '
                        };

static char cap_key_map[] = {   ' ', ' ', '!', '@', '#', '$', '%', '^',  '&', '*', '(', ')',
                                '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',  'T', 'Y', 'U', 'I',
                                'O', 'P', '{', '}', ' ',  ' ', 'A', 'S',  'D', 'F', 'G', 'H',
                                'J', 'K', 'L', ':', '\'', '~', ' ', '\\', 'Z', 'X', 'C', 'V',
                                'B', 'N', 'M', '<', '>',  '?', ' ', ' ',  ' ', ' '
                            };

static int up_key = 0;

void ps2_keyboard_handler(regs_t *regs) {
    uint8_t keycode = inb(0x60);

    char character;

    switch(keycode) {
        case 0xaa: // left shift release
            up_key = 0;
            break;
        case 0x2a: // left shift press
            up_key = 1; 
            break;
        case 0xf: // tab
            character = '\t';
            break;
        case 0xe: // backspace
            character = '\b';
            break;
        case 0x1c:
            character = 0x1c;
            break;
        default:
            if(keycode <= 128) {
                if(up_key) {
                    character = cap_key_map[keycode];
                    break; 
                }  
                character = key_map[keycode];
            }
    }

    write(0, &character, 1);
}
