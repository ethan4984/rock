CC = ../tools/bin/x86_64-elf-ld
NASM = nasm -felf64

build:
	$(NASM) test.asm -o test.o
	$(CC) -shared -fPIC -ffreestanding -nodefaultlibs test.o -T linker.ld -o test.elf
