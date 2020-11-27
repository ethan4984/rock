CC = ../tools/bin/x86_64-elf-gcc
NASM = nasm -felf64

CFLAGS = -ffreestanding -Wall -Wextra -fno-stack-protector -fno-exceptions -Isrc  -Wno-unused-parameter -I. -Iklib -Wno-unused-variable
KERNEL_FLAGS = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer

C_SRC = $(shell find . -type f -name '*.c')

build:
	mkdir -p bin
	$(CC) $(CFLAGS) $(KERNEL_FLAGS) -c $(C_SRC)
	mv *.o bin
	$(NASM) boot.asm -o bin/boot.o
	$(NASM) int/isr.asm -o bin/isr.o
	$(NASM) int/gdt.asm -o bin/gdt_asm.o
	$(NASM) sched/scheduler.asm -o bin/scheduler_asm.o
	nasm -fbin sched/smp.asm -o bin/smp_asm.bin
	$(NASM) real.asm -o bin/real.o
	$(CC) -lgcc -no-pie -nodefaultlibs -nostartfiles -n -T linker.ld -o bin/rock.elf bin/*.o
