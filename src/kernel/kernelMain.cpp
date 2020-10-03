#include <kernel/mm/physicalPageManager.h>
#include <kernel/mm/virtualPageManager.h>
#include <kernel/drivers/keyboard.h>
#include <kernel/fs/ext2/ext2.h>
#include <kernel/drivers/ahci.h>
#include <kernel/drivers/pci.h>
#include <kernel/int/syscall.h>
#include <kernel/sched/task.h>
#include <kernel/sched/hpet.h>
#include <kernel/acpi/madt.h>
#include <kernel/acpi/rsdp.h>
#include <kernel/sched/smp.h>
#include <kernel/mm/kHeap.h>
#include <kernel/int/apic.h>
#include <kernel/int/idt.h>
#include <kernel/int/gdt.h>
#include <kernel/int/tss.h>
#include <kernel/stivale.h>
#include <lib/gui/desktop.h>
#include <lib/gui/widget.h>
#include <kernel/fs/vfs.h>
#include <lib/gui/text.h>
#include <lib/output.h>
#include <lib/vesa.h>
#include <lib/bmp.h> 

#include <stddef.h>

extern "C" void _init();
extern "C" void userTest() asm("userTest");
extern "C" void testDiv();

void task1();
void task2();
void task3();
void task4();
void task5();
void task6();
void task7();

void bruh(uint32_t x, uint32_t y) {
    kprintDS("[KDEBUG]", "you cliked this");
}

extern "C" void kernelMain(stivaleInfo_t *stivaleInfo) {
    _init(); /* calls all global constructors, dont put anything before here */

    pmm::init(stivaleInfo);
    kheap.init();
    virtualPageManager.init();

    idt::init();

    idt::setIDTR();

    gdt::init();

    tssMain.init();
    tssMain.newTss(pmm::alloc(1) + HIGH_VMA); 

    gdt::initCore(0, (uint64_t)tssMain.tss);

    acpi.rsdpInit((uint64_t*)(stivaleInfo->rsdp + HIGH_VMA));

    madtInfo.madtInit();
    madtInfo.printMADT();
    initHPET(); 

    apic::init();

    pci.initPCI();
    ahci.initAHCI();

    initSMP();

    apic::lapicTimerInit(100);

    asm volatile(  "xor %ax, %ax\n"
                   "mov %ax, %fs"
                );

    tasks = (task_t*)kheap.kmalloc(sizeof(task_t) * 0x1000);

    readPartitions(); 

    ext2.init();

    vesa::init(stivaleInfo);

    drawBMP("wallpaper.bmp");

    asm volatile ("sti");

    pannel newPanel(0, 0, 1024 / 8, 2, 0xff);

    widget lmao(400, 400, 3, 3, 0xffff, bruh);

    createWidget(lmao);

    initMouse();

    /*createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task2, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task3, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task4, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task5, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task6, 2);
    createTask(0x10, physicalPageManager.alloc(2) + 0x2000 + HIGH_VMA, 0x8, (uint64_t)task7, 2);*/
    
    for(;;);
}

void task1() {
    uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
        kprintDS("[KDEBUG]", "hi from task0 %d", bruh++);
    }
}


void task2() {
    uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
        kprintDS("[KDEBUG]", "hi from task1 %d", bruh++);
    }
}

void task3() {
    uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
        kprintDS("[KDEBUG]", "hi from task2 %d", bruh++);
    }
}

void task4() {
    uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
        kprintDS("[KDEBUG]", "hi from task3 %d", bruh++);
    }
}

void task5() {
    uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
        kprintDS("[KDEBUG]", "hi from task4 %d", bruh++);
    }
}

void task6() {
    uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
        kprintDS("[KDEBUG]", "hi from task5 %d", bruh++);
    }
}

void task7() {
    uint64_t bruh = 0;
    while(1) {
        for(uint64_t i = 0; i < 100000000; i++);
        kprintDS("[KDEBUG]", "hi from task6 %d", bruh++);
    }
}
