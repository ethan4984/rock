#include <shitio.h>
#include <scheduler.h>
#include <paging.h>
#include <process.h>
#include <vector.h>

using namespace standardout;
using namespace MM;

vector<process> obj;

uint64_t num_of_proc = 0;
uint64_t *running;
uint64_t current_process = 0;

void request_new_process(uint64_t range, void (*entry_function)())
{
    process new_process(range, entry_function);

    obj.push_back(new_process);

    t_print("Hmm");
}

volatile int timer_ticks = 0;
volatile int seconds = 0;

void PITI()
{
    outb(0x20, 0x20);
    timer_ticks++;
    if(timer_ticks % 94 == 0)
        ++seconds;

    /*static int bruh = 0;
    if(!bruh) {
		obj.at(0).save_regs();
		void (*main_func)() = obj.at(1).entry_point;
        (*main_func)();
        bruh++;
	}
	if(bruh == 1) {
		obj.at(1).save_regs();
		obj.at(0).restore();
	}*/
}

void sleep(volatile int ticks)
{
    seconds = 0;
    while(seconds < ticks);
}
