#include <string.hpp>

struct symlist_entry {
    size_t addr;
    lib::string name;
};

symlist_entry symlist[] = {
    { 0xffffffffffffffff, "" }
}
