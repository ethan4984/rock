#ifndef BITMAP_HPP_
#define BITMAP_HPP_

#include <cstddef>
#include <cstdint>
#include <types.hpp>

namespace lib {

class bitmap {
public:
    bitmap(size_t inital_size, bool can_grow = false);
    bitmap() = default;

    ssize_t alloc();
    void free(size_t index);
private:
    uint8_t *bm;
    size_t bm_size;
    bool can_grow;
};

}

#endif
