CC = ~/opt64/cross/bin/x86_64-elf-g++
AS = ~/opt64/cross/bin/x86_64-elf-as
NASM = nasm -felf64

NORMAL_CFLAGS = -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -fno-exceptions -IKernel -Ilibc++
KERNEL_CRAP = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics
CFLAGS = $(NORMAL_CFLAGS) $(KERNEL_CRAP)

SCRC = $(shell find . -type f -name '*.cpp')
SRCA = $(shell find . -type f -name '*.asm')
SRCS = $(shell find . -type f -name '*.s')
OBJ = alloc.o graphics.o interrupt.o memory.o keyboard.o kernel_init.o paging.o port.o process.o scheduler.o shell.o shitio.o boot.o exceptions.o interrupts.o string.o sound.o
QEMUFLAGS = -smp cpus=4 -m 4G -vga vmware -serial file:serial.log -soundhw pcspk


CRTI_OBJ=crti.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=crtn.o

OBJ_LINK_LIST:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)
INTERNAL_OBJS:=$(CRTI_OBJ) $(OBJ) $(CRTN_OBJ)

kernel: compile
	$(CC) -lgcc -nodefaultlibs -nostartfiles -n -T link.ld -o iso/boot/crepOS.bin $(OBJ_LINK_LIST)
	grub-mkrescue -o crepOS.iso iso
	rm $(OBJ) crtn.o crti.o

compile:
	$(CC) $(SCRC) $(CFLAGS) -c
	$(NASM) Kernel/boot.asm -o boot.o
	$(NASM) Kernel/exceptions.asm -o exceptions.o
	$(NASM) Kernel/interupts.asm -o interrupts.o
	$(AS) Kernel/crtn.s -o crtn.o
	$(AS) Kernel/crti.s -o crti.o

qemu: kernel
	touch serial.log
	qemu-system-x86_64 $(QEMUFLAGS) -cdrom crepOS.iso &
	tail -n0 -f serial.log

qemuinfo: kernel
	qemu-system-x86_64 -smp cpus=4 -cdrom crepOS.iso -m 4G -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware

qemudebug: kernel
	qemu-system-x86_64 -smp cpus=4 -cdrom crepOS.iso -m 4G -no-reboot -monitor stdio -d int -no-shutdown -vga vmware

clean:
	rm $(OBJ)
