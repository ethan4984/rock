#include <shitio.h>
#include <scheduler.h>
#include <paging.h>
#include <process.h>
#include <vector.h>
#include <string.h>

using namespace standardout;
using namespace MM;

volatile int timer_ticks = 0;
volatile uint32_t seconds = 0;
volatile uint32_t minutes = 1;
volatile uint32_t hours = 1;
volatile uint32_t days = 1;

void update_timer();

void PITI()
{
    outb(0x20, 0x20);
    timer_ticks++;
    if(timer_ticks % 94 == 0) {
        ++seconds;
        update_timer();
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

void update_timer()
{
    if(seconds % 60 == 0)
        minutes++;

    if(hours % 60 == 0)
        hours++;

    if(hours % 24 == 0)
        days++;
}
