#pragma once

#include <stdint.h>
#include <stdbool.h>

namespace kernel {

struct file_t {
    char fileName[256];
    uint64_t fileSize;
};

class directory_t {
public:
    bool fileExists();
private:
    file_t *files;
    uint64_t fileCount;
};

class vfs_t { 
public:
    void vfsInit();
private:
    directory_t *directories;
};

inline vfs_t vfs;

}
