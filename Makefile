CC=i686-elf-g++
AS=i686-elf-as

srcC = Kernel/kernel.cpp Kernel/shitio.cpp Kernel/port.cpp Kernel/interrupt.cpp
srcO = Bin/kernel.o Bin/port.o Bin/shitio.o Bin/irt.o Bin/interrupt.o
CFLAGS = -c -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector

install:
	nasm -f elf32 Kernel/boot.asm -o Bin/boot.o
	nasm -f elf32 Kernel/interupts.asm -o Bin/irt.o
	$(CC) $(srcC) $(CFLAGS)
	mv port.o Bin
	mv shitio.o Bin
	mv kernel.o Bin
	mv interrupt.o Bin
clean:
	rm $(srcO)
	rm $(srcASO)
	rm iso/boot/crep.bin crepOS.iso
iso: install
	$(CC) -T link.ld -o iso/boot/shitOS.bin -ffreestanding -O2 -nostdlib Bin/boot.o $(srcO) -lgcc
	grub-mkrescue -o crepOS.iso iso
qemu: iso
	qemu-system-i386 -serial file:serial.log crepOS.iso &
	tail -n0 -f serial.log
