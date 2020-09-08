#pragma once

#include <lib/memoryUtils.h>

namespace kernel {

struct command {
    command(const char *baseIdentifier, function<void, char*, char*> handler) : identifier(baseIdentifier), handler(handler) { }

    command() { }

    const char *identifier;

    function<void, char *, char *> handler;
};

void initCommands();

}
