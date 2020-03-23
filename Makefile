CC = ~/opt64/cross/bin/x86_64-elf-g++
CFLAGS = -c -std=c++17 -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -lstdc++ -fno-exceptions -IKernel -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics

srcC = Kernel/kernel_init.cpp Kernel/shitio.cpp Kernel/port.cpp Kernel/interrupt.cpp Kernel/keyboard.cpp Kernel/memory.cpp Kernel/graphics.cpp Kernel/shell.cpp Kernel/paging.cpp Kernel/process.cpp Kernel/scheduler.cpp
srcO = Bin/kernel_init.o Bin/port.o Bin/shitio.o Bin/interrupt.o Bin/keyboard.o Bin/memory.o Bin/graphics.o Bin/shell.o Bin/paging.o Bin/process.o Bin/scheduler.o

kernel: compile
	~/opt64/cross/bin/x86_64-elf-g++ -n -T link.ld -o iso/boot/crepOS.bin -ffreestanding -O2 -nostdlib Bin/boot.o Bin/exceptions.o Bin/irt.o $(srcO) -lgcc
		grub-mkrescue -o crepOS.iso iso

compile:
	nasm -felf64 Kernel/boot.asm -o Bin/boot.o
	nasm -felf64 Kernel/exceptions.asm -o Bin/exceptions.o
	nasm -felf64 Kernel/interupts.asm -o Bin/irt.o
	$(CC) $(CFLAGS) $(srcC)
	mv shitio.o Bin
	mv port.o Bin
	mv kernel_init.o Bin
	mv interrupt.o Bin
	mv keyboard.o Bin
	mv memory.o Bin
	mv graphics.o Bin
	mv shell.o Bin
	mv paging.o Bin
	mv process.o Bin
	mv scheduler.o Bin

qemu: kernel
	touch serial.log
	qemu-system-x86_64 -smp cpus=4 -cdrom crepOS.iso -m 4G -vga vmware -serial file:serial.log &
	tail -n0 -f serial.log

qemuinfo: kernel
	qemu-system-x86_64 -smp cpus=4 -cdrom crepOS.iso -m 4G -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware
qemudebug: kernel
	qemu-system-x86_64 -smp cpus=4 -cdrom crepOS.iso -m 4G -no-reboot -monitor stdio -d int -no-shutdown -vga vmware
