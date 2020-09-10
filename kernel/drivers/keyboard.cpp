#include <kernel/drivers/keyboard.h>
#include <kernel/mm/kHeap.h>
#include <lib/memoryUtils.h>
#include <lib/asmUtils.h>
#include <lib/output.h>

#include <stdbool.h>

namespace kernel {

void dummyChar(uint8_t c) {

}

function<void, uint8_t> func = dummyChar;
	
char keyMap[] = {   ' ', ' ', '1', '2', '3',  '4', '5', '6',  '7', '8', '9', '0',
                    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
                    'o', 'p', '[', ']', ' ',  ' ', 'a', 's',  'd', 'f', 'g', 'h',
                    'j', 'k', 'l', ';', '\'', '`', ' ', '\\', 'z', 'x', 'c', 'v',
                    'b', 'n', 'm', ',', '.',  '/', ' ', ' ',  ' ', ' '
                };

char capKeyMap[] = {    ' ', ' ', '!', '@', '#', '$', '%', '^',  '&', '*', '(', ')',
                        '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',  'T', 'Y', 'U', 'I',
                        'O', 'P', '{', '}', ' ',  ' ', 'A', 'S',  'D', 'F', 'G', 'H',
                        'J', 'K', 'L', ':', '\'', '~', ' ', '\\', 'Z', 'X', 'C', 'V',
                        'B', 'N', 'M', '<', '>',  '?', ' ', ' ',  ' ', ' '
                   };
				   
static bool upkey = false;

void keyboardHandlerMain(regs_t *regs) {
    uint8_t keycode = inb(0x60);

    switch(keycode) {
        case 0xaa: // left shift release
            upkey = false;
            break;
        case 0x2a: // left shift press
            upkey = true;
            break;
        case 0xf: // tab
            for(int i = 0; i < 4; i++)
                func(' ');
            break;
        case 0xe: // backspace
            func('\b');
            break;
        case 0x1c:
            func(0x1c);
            break;
        default:
            if(keycode <= 128) {
                if(upkey) {
                    func(capKeyMap[keycode]);
                    break; 
                }  
                func(keyMap[keycode]);
            }
    } 
}

void changeInputFunction(function<void, uint8_t> newFunc) {
    func = newFunc;
}

}
