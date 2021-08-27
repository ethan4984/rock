#include <string.hpp>

struct symlist_entry {
    size_t addr;
    lib::string name;
};

__attribute__((section(".sym")))
symlist_entry symlist[] = {
    { 0xffffffffffffffff, "" }
};
