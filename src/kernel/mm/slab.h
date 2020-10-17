#pragma once

namespace mm {

class slabHeap {
public:
    slabHeap(uint64_t pageCnt);
    ~slabHeap();

    void *alloc(size_t cnt);
    uint64_t free(void *addr);
    void *realloc(void *addr, size_t cnt);
private:
    bitmapHeap bitmap;
    bitmapHeap *caches;
};

}
