CC=i686-elf-g++
AS=i686-elf-as

srcC = Kernel/kernel.cpp Kernel/shitio.cpp Kernel/port.cpp Kernel/interrupt.cpp Kernel/keyboard.cpp Kernel/memory.cpp Kernel/shell.cpp Kernel/paging.cpp
srcO = Bin/kernel.o Bin/port.o Bin/shitio.o Bin/interrupt.o Bin/keyboard.o Bin/memory.o Bin/shell.o Bin/paging.o
CFLAGS = -c -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector

install:
	nasm -f elf32 Kernel/boot.asm -o Bin/boot.o
	nasm -f elf32 Kernel/interupts.asm -o Bin/irt.o
	nasm -f elf32 Kernel/page.asm -o Bin/page.o
	$(CC) $(srcC) $(CFLAGS)
	mv port.o Bin
	mv shitio.o Bin
	mv kernel.o Bin
	mv interrupt.o Bin
	mv keyboard.o Bin
	mv memory.o Bin
	mv shell.o Bin
	mv paging.o Bin
clean:
	rm $(srcO)
	rm $(srcASO)
	rm iso/boot/crep.bin crepOS.iso
iso: install
	$(CC) -T link.ld -o iso/boot/crep.bin -ffreestanding -O2 -nostdlib Bin/boot.o Bin/irt.o Bin/page.o $(srcO) -lgcc
	grub-mkrescue -o crepOS.iso iso
qemu: iso
	touch serial.log
	qemu-system-i386 -enable-kvm -serial file:serial.log crepOS.iso &
	tail -n0 -f serial.log
