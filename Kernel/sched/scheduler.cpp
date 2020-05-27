#include <Kernel/sched/scheduler.h>
#include <Kernel/sched/task.h>
#include <Kernel/sched/thread.h>
#include <Kernel/mm/memHandler.h>
#include <Slib/videoIO.h>
#include <Slib/memoryUtils.h>
#include <Slib/ports.h>

using namespace out;

volatile uint64_t timerTicks = 0;
volatile uint64_t seconds = 0;

uint64_t contextSwitchCounter = 0, PITfrequency = 0, totalTasks = 0, currentTask = 0;
int bitmapSize = 0;
bool flowUp = true;
uint8_t *taskBitmap;

task_t *Tasks;

extern "C" void startTask(uint64_t stack, uint64_t task) asm("startTask");
extern "C" void switchTask(uint64_t rsp, uint64_t rbp) asm("switchTask");

uint64_t firstFreeTaskSlot();

extern "C" void schedulerMain(uint64_t lastRBP, uint64_t lastRSP) 
{
    outb(0x20, 0x20); // EOI   

    if(++timerTicks % PITfrequency == 0) {
        seconds++;
        contextSwitchCounter = 0;
    }

    if(totalTasks >= 1) {
        task_t *oldTask = &Tasks[currentTask];

        if(oldTask->status == RUNNING) { /* saves the state of the oldTask */
            oldTask->rbp = lastRBP;
            oldTask->rsp = lastRSP;
        }

        if(flowUp) { 
            bool found = false;
            for(int i = currentTask + 1; i < bitmapSize; i++) { /* finds next task to run going upwards */
                if(isset(taskBitmap, i)) {
                    currentTask = i;
                    found = true;
                    break;
                } 
            }
            if(!found) 
                flowUp = false;
        } else {
            bool found = false;
            for(int i = currentTask - 1; i >= 0; i--) { /* finds next task to run going down */
                if(isset(taskBitmap, i)) {
                    currentTask = i;
                    found = true; 
                    break;
                }
            }
            if(!found)
                flowUp = true;
        }

        task_t *task2run = &Tasks[currentTask];

        if(task2run->status == WAITING2START) { /* If the task has not already started start it */
            task2run->status = RUNNING;
            contextSwitchCounter++;
            startTask(task2run->rsp, (uint64_t)task2run->mainMethod);
        }

        contextSwitchCounter++;
        switchTask(task2run->rsp, task2run->rbp);
    }
}

void initTask(task_t newTask)
{
    static uint64_t maxTasks = 5;
    if(maxTasks >= totalTasks) {
        Tasks = (task_t*)realloc(Tasks, sizeof(task_t) * (maxTasks + 5));
        maxTasks += 5;
    }
    uint64_t place = firstFreeTaskSlot();
    set(taskBitmap, place);
    cPrint("Task slot %d", place);
    Tasks[place] = newTask;
    totalTasks++;
}

void initScheduler() 
{
    Tasks = (task_t*)malloc(sizeof(task_t) * 5); 
    taskBitmap = (uint8_t*)malloc(32); 
    memset(taskBitmap, 0, 32);
    bitmapSize = 32 * 8;
}

void sleep(volatile uint64_t waitTime) /* waitTime is in seconds */
{
    seconds = 0;
    while(seconds < waitTime);
}

void startCounter(int frequency, uint8_t counter, uint8_t mode)
{
    if(!frequency)
        return;

    PITfrequency = frequency;

    uint16_t divisor = 1193181 / frequency;

    uint8_t ossal = 0;
    ossal = (ossal & ~0xe) | mode;
    ossal = (ossal & ~0x30) | 0xe;
    ossal = (ossal & ~0xc0) | counter;
    outb(0x43, ossal);

    outb(0x40, (uint8_t)(divisor & 0xff));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xff));
}

uint64_t firstFreeTaskSlot() 
{
    for(int i = 0; i < bitmapSize; i++) {
        if(!isset(taskBitmap, i)) { 
            return i; 
        }
    }
    return bitmapSize + 1;
}

void killTask(uint64_t taskIndex) 
{
    clear(taskBitmap, taskIndex);
}
