#include <shitio.h>
#include <scheduler.h>
#include <paging.h> 
#include <process.h>

using namespace standardout;
using namespace MM;

static uint64_t num_of_processes = 0;

volatile int timer_ticks = 0;
volatile int seconds = 0;

void PITI()
{
	static uint64_t current_process = 0;
	
    outb(0x20, 0x20);
    timer_ticks++;
    if(timer_ticks % 94 == 0) {
        ++seconds;
        
    }  
}

void sleep(volatile int ticks)
{
    seconds = 0;
    while(seconds < ticks);
}
 
