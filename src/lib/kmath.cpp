#include <lib/kmath.h>

uint64_t pow(uint64_t base, uint64_t expo) {
    uint64_t ret = base;
    for(size_t i = 0; i < expo; i++, ret *= ret);
    return ret;
}
