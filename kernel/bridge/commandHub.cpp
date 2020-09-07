#include <kernel/bridge/commandHub.h>
#include <kernel/bridge/kterm.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

namespace kernel {

void kterm::commandHub() {
    for(uint32_t i = 0; i < commandCnt; i++) {
        if(strcmp(commands[i].identifier, buffer) == 0) {
            commands[i].handler(buffer);
        }
    }
}

}
