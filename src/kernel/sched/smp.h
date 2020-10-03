#pragma once

#include <stdint.h>

struct cpuInfo_t { 
    uint64_t coreID;
    uint64_t numberOfTasks; 
    int64_t currentTask;
    static uint8_t numberOfCores;
};

void initSMP();

void spinLock(char *ptr);

void spinRelease(char *ptr);

inline cpuInfo_t *cpuInfo;
