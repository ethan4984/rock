#include <Kernel/sched/task.h>
#include <Kernel/sched/scheduler.h>
#include <Kernel/mm/memHandler.h>
#include <Slib/memoryUtils.h>
#include <Slib/videoIO.h>

using namespace out;

task_t::task_t(void *mainMethod_t, void *stackBase_t, uint64_t stackLimit_t, uint8_t status_t)
	: stackBase(stackBase_t), mainMethod(mainMethod_t), stackLimit(stackLimit_t), status(status_t)
{
	threadBitmap = (uint8_t*)malloc(10); /* allocates 10 bytes aka 80 bits or 80 threads, if we need more we realloc */
	threads = (thread*)malloc(sizeof(thread) * 5); /* allocates 5 threads, if need more realloc */
    rsp = (uint64_t)stackBase_t;
    rbp = (uint64_t)stackBase_t;
}

uint64_t task_t::firstFreeThreadSlot(uint64_t startOverride = 0)
{
	for(uint64_t i = startOverride; i < bitmapSize * 8; i++) {
		if(!isset(threadBitmap, i))
			return i;
	}
	return bitmapSize * 8 + 1; 
}

void task_t::addThread(void *stack, void *main) 
{
    static uint64_t maxThreads = 5;
    thread newThread(WAITING2START, (uint64_t)stack, (uint64_t)stack, main);
    if(firstFreeThreadSlot() > maxThreads) {
        threads = (thread*)realloc(threads, sizeof(thread) * 5);
        maxThreads += 5;
    }
    threads[totalThreads++] = newThread;
}

void task_t::createThread(void *main, uint64_t stackSizeOverride = 0x800) /* default stack size = 2 kb */
{
    void *stack = malloc(stackSizeOverride);
    addThread(stack, main);
    cPrint("\nNew thread: [stack] %x [main] %x\n", stack, main);
}

void task_t::killThread(uint64_t threadIndex) /* TODO: Free the heap */
{
	clear(threadBitmap, threadIndex);
	memset(stackBase, 0, stackLimit); /* zeros out the stack */
	free(stackBase);
} 
