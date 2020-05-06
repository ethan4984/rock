#include <shitio.h>
#include <scheduler.h>
#include <paging.h>
#include <process.h>
#include <vector.h>
#include <string.h>

using namespace standardout;
using namespace MM;

volatile int timer_ticks = 0;
uint64_t total_tasks = 0;

volatile uint32_t seconds = 0;

static volatile int current_task;
task *tasks;

uint64_t context_switch_counter = 0;

void init_scheduler()
{
    tasks = (task*)malloc(sizeof(task)*10); // default support for 10 tasks
}

void add_task(uint8_t *stack, void *main)
{
    static int max_tasks = 10;
	task task_entry;
	task_entry.status = WAITING_TO_START;
	task_entry.rbp = (uint64_t)stack; //creates a new stack
	task_entry.rsp = (uint64_t)stack;
	task_entry.task = main;

    if(max_tasks + 1 == total_tasks) {
        max_tasks += 10;
        free(tasks);
        tasks = (task*)malloc(sizeof(tasks) * max_tasks);
    }
	tasks[total_tasks++] = task_entry;
}

extern "C" void PITI(uint64_t last_rbp, uint64_t last_rsp)
{
    outb(0x20, 0x20);
    timer_ticks++;
    if(timer_ticks % 1000 == 0) {
        ++seconds;
        context_switch_counter = 0;
    }

    if(total_tasks >= 1) {
        int last_task = current_task;
        int task_index = ++current_task;
        task_index %= total_tasks;
        current_task = task_index;

        task *old_task = &tasks[last_task];

        if(old_task->status == RUNNING) { // saves the state of the running task
            old_task->rbp = last_rbp;
            old_task->rsp = last_rsp;
        }

        task *task_to_run = &tasks[current_task];

        if (task_to_run->status == WAITING_TO_START) {
            task_to_run->status = RUNNING;
            context_switch_counter++;
            start_process(task_to_run->rsp, (uint64_t)task_to_run->task);
        }

        // assuming it has to be waiting

        context_switch_counter++;
        switch_process(task_to_run->rsp, task_to_run->rbp);
    }
}

void sleep(volatile int ticks)
{
    seconds = 0;
    while(seconds < ticks);
}

void nanosleep(volatile uint32_t offset)
{
    timer_ticks = 0;
    while(timer_ticks < offset);
}
