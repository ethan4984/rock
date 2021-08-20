#include <bitmap.hpp>
#include <mm/slab.hpp>

namespace lib {

bitmap::bitmap(size_t inital_size, bool can_grow) : bm_size(inital_size), can_grow(can_grow) {
    bm = (uint8_t*)kmm::calloc(div_roundup(inital_size, 8));
}

ssize_t bitmap::alloc() {
    for(size_t i = 0; i < bm_size; i++) {
        if(!bm_test(bm, i)) {
            bm_set(bm, i);
            return i;
        }
    }
    return -1;
}

void bitmap::free(size_t index) {
    if(index >= bm_size)
        return;
    bm_clear(bm, index);
}

}
