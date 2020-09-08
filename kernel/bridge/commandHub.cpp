#include <kernel/bridge/commandHub.h>
#include <kernel/bridge/kterm.h>
#include <kernel/bridge/cat.h>
#include <kernel/bridge/ls.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

namespace kernel {

void kterm::commandHub() {
    char identifier[50] = { 0 };

    uint32_t loc = strlen(buffer);
    for(uint32_t i = 0; i < strlen(buffer); i++) {
        if(buffer[i] == ' ') {
            loc = i; 
        }
    }

    strncpy(identifier, buffer, loc);

    for(uint32_t i = 0; i < commandCnt; i++) {
        if(strcmp(commands[i].identifier, identifier) == 0) { 
            putchar('\n');
            commands[i].handler(buffer, path);
            goto end;
        }
    }
    
    if(*identifier != '\0')
        print("\n\"%s\" command not found\n", buffer);
    else
        putchar('\n');

end:
    print("[root %s ] > ", path);
}

void kterm::updatePath(const char *path) {
    
}

void initCommands() {
    command newCommand("cd", cdHandler);
    kterm.addCommand(newCommand);
    command newCommand1("ls", lsHandler);
    kterm.addCommand(newCommand1);
} 

}
