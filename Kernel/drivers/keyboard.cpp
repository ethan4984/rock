#include <Kernel/drivers/keyboard.h>
#include <Kernel/mm/memHandler.h>
#include <Kernel/int/pic.h>
#include <Slib/ports.h>
#include <Slib/videoIO.h>

#include <stdint.h>
#include <stddef.h>

using namespace out;

char keyMap[] =       {
                            ' ', ' ', '1', '2', '3',  '4', '5', '6',  '7', '8', '9', '0',
                            '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
                            'o', 'p', '[', ']', ' ',  ' ', 'a', 's',  'd', 'f', 'g', 'h',
                            'j', 'k', 'l', ';', '\'', '`', ' ', '\\', 'z', 'x', 'c', 'v',
                            'b', 'n', 'm', ',', '.',  '/', ' ', ' ',  ' ', ' '
                      };

char capKeyMap[] =  {
                            ' ', ' ', '!', '@', '#', '$', '%', '^',  '&', '*', '(', ')',
                            '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',  'T', 'Y', 'U', 'I',
                            'O', 'P', '{', '}', ' ',  ' ', 'A', 'S',  'D', 'F', 'G', 'H',
                            'J', 'K', 'L', ':', '\'', '~', ' ', '\\', 'Z', 'X', 'C', 'V',
                            'B', 'N', 'M', '<', '>',  '?', ' ', ' ',  ' ', ' '
                    };

char *keyBuffer;
uint32_t bufferSize = 0, bufferIndex = 0;

bool upKey = false;

void grabKeys(char *newBuffer) 
{
    clearGate(33); // unmask irq1
    newBuffer = (char*)malloc(32); 
    keyBuffer = newBuffer;
    bufferSize = 32;
}

void addCharacter(char character) 
{
    if(bufferIndex == bufferSize - 1) {
        keyBuffer = (char*)realloc(keyBuffer, 32);
        bufferSize += 32;
    }
    keyBuffer[bufferIndex] = character;
}

void keyboardHandler()
{
    cPrint("bruh");
    clearGate(33); // unmask irq1

    if(bufferSize != 0) {
        uint8_t keycode = inb(0x60); // grab keycode through ps2 port 0x60

        switch(keycode) {
            case 0x1c:  // enter
                bufferSize = 0;
                maskGate(33); // mask irq1 
                break;
            case 0xe:  // backspace
                if(bufferIndex != 0) 
                    keyBuffer[bufferIndex--] = 0; 
                break;
            case 0xf: // tab
                break;
            case 0x2a: // Left Shift Press
                upKey = true;
                break;
            case 0xaa: // Left Shift Release
                upKey = false;
                break;
            default:
                if(keycode <= 128) {
                    if(upKey) {
                        addCharacter(capKeyMap[keycode]);
                        cPrint("%c", capKeyMap[keycode]);
                        bufferIndex++;
                        break;
                    }

                    cPrint("%c", keyMap[keycode]);
                    addCharacter(keyMap[keycode]);
                    bufferIndex++;
                }
        }
    }
}
