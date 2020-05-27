CC = ~/opt/cross/bin/x86_64-elf-g++

NORMAL_CFLAGS = -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -fno-exceptions -I../Kernel -I../Slib
KERNEL_CRAP = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics
CFLAGS = $(NORMAL_CFLAGS) $(KERNEL_CRAP)

SCRC = $(shell find . -type f -name '*.cpp')

all:
	$(CC) $(CFLAGS) -c $(SCRC) 
	mv *.o ../Bin
