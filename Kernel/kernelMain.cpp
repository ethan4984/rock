#include <Kernel/mm/memoryParse.h>
#include <Kernel/mm/physicalPageHandler.h>
#include <Kernel/mm/memHandler.h>
#include <Kernel/int/pic.h>
#include <Kernel/int/madt.h>
#include <Kernel/sched/scheduler.h>
#include <Kernel/acpiUtils.h> 
#include <Kernel/drivers/vesa.h>
#include <Kernel/drivers/keyboard.h>
#include <Kernel/drivers/mouse.h>
#include <Kernel/pci.h>
#include <Slib/videoIO.h>
#include <Slib/memoryUtils.h>
#include <Slib/stringUtils.h>
#include <Apps/prompt.h>

using namespace out;

extern void _init() asm("_init");
extern void testDiv() asm("testDiv");

void pmmTest();
void allocTest();

void testTask1();
void testTask2();
void testTask3();
void testTask4();
void testTask5();
void testTask6();



void kernelTask()
{
/*    task_t task1((void*)testTask1, malloc(0x400), 0x400, WAITING2START);
    task_t task2((void*)testTask2, malloc(0x400), 0x400, WAITING2START);
    task_t task3((void*)testTask3, malloc(0x400), 0x400, WAITING2START);
    task_t task4((void*)testTask4, malloc(0x400), 0x400, WAITING2START);
    task_t task5((void*)testTask5, malloc(0x400), 0x400, WAITING2START);
    task_t task6((void*)testTask6, malloc(0x400), 0x400, WAITING2START);

    initTask(task1); 
    initTask(task2); 
    initTask(task3); 
    initTask(task4);
    initTask(task5);
    initTask(task6);*/

    //widget bruh(400, 400, 10, 10, 0xFF0000);
    
    task_t prompt((void*)promptMain, malloc(0x400), 0x400, WAITING2START);

    initTask(prompt);

    putchar('\n');
   
    for(;;);
}

stivaleInfo_t *stivaleInfo;

extern "C" void kernelMain(stivaleInfo_t *bootInfo) 
{
    _init(); /* reqiured for global constructors. PS, be careful about calling shit before this */

    initVesa(bootInfo);

    initalizeVESA(0xffffffff, 0, bootInfo->framebufferWidth, bootInfo->framebufferHeight);

    memInit(bootInfo);

    idtInit();

    initAcpi();

    madtParse();

    pciInit();

    startCounter(1000, 0, 6);

    asm volatile ("sti");

    initScheduler();

    task_t mainKernelTask((void*)kernelTask, malloc(0x400), 0x400, WAITING2START);

    initTask(mainKernelTask);

    stivaleInfo = bootInfo;

    for(;;);
}

stivaleInfo_t *getstivale()
{
    return stivaleInfo;
}

void pmmTest() 
{
    uint64_t *page1 = (uint64_t*)physicalPageAlloc(1);
    uint64_t *page2 = (uint64_t*)physicalPageAlloc(1);
    uint64_t *page3 = (uint64_t*)physicalPageAlloc(2);
    uint64_t *page4 = (uint64_t*)physicalPageAlloc(1);
    uint64_t *page5 = (uint64_t*)physicalPageAlloc(1);

    physicalPageFree(page3);

    uint64_t *page6 = (uint64_t*)physicalPageAlloc(3);
    uint64_t *page7 = (uint64_t*)physicalPageAlloc(1);
    uint64_t *page8 = (uint64_t*)physicalPageAlloc(1);
    uint64_t *page9 = (uint64_t*)physicalPageAlloc(1);

    cPrint("Address of page1 %x", page1);
    cPrint("Address of page2 %x", page2);
    cPrint("Address of page3 %x", page3);
    cPrint("Address of page4 %x", page4);
    cPrint("Address of page5 %x", page5);
    cPrint("Address of page6 %x", page6);
    cPrint("Address of page7 %x", page7);
    cPrint("Address of page8 %x", page8);
    cPrint("Address of page9 %x", page9);
}

void allocTest()
{
    uint16_t *testFree1 = (uint16_t*)malloc(4);
    free(testFree1);
    uint16_t *testFree2 = (uint16_t*)malloc(2);
    uint16_t *testFree3 = (uint16_t*)malloc(8);
    free(testFree3);
    uint16_t *testFree4 = (uint16_t*)malloc(9);
    uint16_t *testFree5 = (uint16_t*)malloc(32);
    uint16_t *testFree6 = (uint16_t*)malloc(32);

    free(testFree5);

    uint16_t *testFree7 = (uint16_t*)malloc(32);
    uint16_t *testFree8 = (uint16_t*)malloc(3);


    cPrint("Address of testFree1 %x", testFree1);
    cPrint("Address of testFree2 %x", testFree2);
    cPrint("Address of testFree3 %x", testFree3);
    cPrint("Address of testFree4 %x", testFree4);
    cPrint("Address of testFree5 %x", testFree5);
    cPrint("Address of testFree6 %x", testFree6);
    cPrint("Address of testFree7 %x", testFree7);
    cPrint("Address of testFree8 %x", testFree8);
}

void testTask1() 
{
    uint64_t cnt = 0;
    while(1) {
        cPrint("Task1 %d", cnt++);
        for(int i = 0; i < 10000; i++) {
            asm volatile ("NOP");
        }
    }
}

void testTask2() 
{
    uint64_t cnt = 0;
    while(1) {
        cnt += 10;
        cPrint("Task2 %d", cnt);
        for(int i = 0; i < 10000; i++) {
            asm volatile ("NOP");
        }
    }
}

void testTask3() 
{
    uint64_t cnt = 0;
    while(1) {
        cnt += 69;
        cPrint("Task3 %d", cnt);
        for(int i = 0; i < 10000; i++) {
            asm volatile ("NOP");
        }
    }
}

void testTask4() 
{
    uint64_t cnt = 0;
    while(1) {
        cnt += 69;
        cPrint("Task4 %d", cnt);
        for(int i = 0; i < 10000; i++) {
            asm volatile ("NOP");
        }
    }
}

void testTask5() 
{
    uint64_t cnt = 0;
    while(1) {
        cnt += 69;
        cPrint("Task5 %d", cnt);
        for(int i = 0; i < 10000; i++) {
            asm volatile ("NOP");
        }
    }
}

void testTask6() 
{
    uint64_t cnt = 0;
    while(1) {
        cnt += 69;
        cPrint("Task6 %d", cnt);
        for(int i = 0; i < 10000; i++) {
            asm volatile ("NOP");
        }
    }
}
