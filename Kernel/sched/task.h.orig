#pragma once

#include <stdint.h>
#include <Kernel/sched/thread.h>

class task_t 
{
    public:
        task_t(void *mainMethod_t, void *stackBase_t, uint64_t stackLimit_t, uint8_t status_t);

        void *stackBase;

        void *mainMethod;

        uint64_t stackLimit;

        uint8_t status;
        
        uint64_t grabNextThread();
        
        void createThread(void *main, uint64_t stackSizeOverride);
        
        void killCurrentTask();

        void killThread(uint64_t index);

        uint64_t rbp, rsp;
    private: 
        thread *threads;

        uint8_t *threadBitmap; /* this bitmap keeps track of all the threads */
        
		uint64_t bitmapSize = 0, totalThreads = 0;
        
        void addThread(void *stack, void *main);
        
        uint64_t firstFreeThreadSlot(uint64_t startOverride);
};
