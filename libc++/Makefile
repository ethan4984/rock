CC = ~/opt64/cross/bin/x86_64-elf-g++
AS = ~/opt64/cross/bin/x86_64-elf-as

NORMAL_CFLAGS = -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -fno-exceptions -I../Kernel -I../libc++
KERNEL_CRAP = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics
CFLAGS = $(NORMAL_CFLAGS) $(KERNEL_CRAP)

SCRC = $(shell find . -type f -name '*.cpp')
OBJ = ../Bin/alloc.o ../Bin/graphics.o ../Bin/interrupt.o ../Bin/memory.o ../Bin/keyboard.o ../Bin/kernel_init.o ../Bin/paging.o ../Bin/port.o ../Bin/process.o ../Bin/scheduler.o ../Bin/shell.o ../Bin/shitio.o ../Bin/boot.o ../Bin/exceptions.o ../Bin/interrupts.o ../Bin/string.o ../Bin/sound.o ../Bin/acpi.o

default:
	$(CC) $(SCRC) $(CFLAGS) -c
	mv string.o ../Bin
