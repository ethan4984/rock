#pragma once

#include <kernel/fs/ext2/types.h>

namespace ext2 {

void init(int partition);

size_t read(const char *path, uint64_t start, uint64_t cnt, void *buffer, int partition);

void write(const char *path, uint64_t start, uint64_t cnt, void *buffer, int partition);

BGD_t readBGD(uint64_t index, int partition);

}
