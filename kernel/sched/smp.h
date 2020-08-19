#pragma once

#include <stdint.h>

namespace kernel {

struct cpuInfo_t { 
    uint64_t coreID;
    uint64_t numberOfTasks; 
    int64_t currentTask;
};

void initSMP();

inline cpuInfo_t *cpuInfo;

}
