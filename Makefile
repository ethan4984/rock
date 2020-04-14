CC = ~/opt64/cross/bin/x86_64-elf-g++
AS = ~/opt64/cross/bin/x86_64-elf-as
NASM = nasm -felf64

NORMAL_CFLAGS = -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -fno-exceptions -IKernel -Ilibc++
KERNEL_CRAP = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics
CFLAGS = $(NORMAL_CFLAGS) $(KERNEL_CRAP)

SCRC = $(shell find . -type f -name '*.cpp')
SRCA = $(shell find . -type f -name '*.asm')
SRCS = $(shell find . -type f -name '*.s')
OBJ = alloc.o graphics.o interrupt.o memory.o keyboard.o kernel_init.o paging.o port.o process.o scheduler.o shell.o shitio.o boot.o exceptions.o interrupts.o string.o sound.o acpi.o
QEMUFLAGS = -smp cpus=4 -m 4G -vga vmware -serial file:serial.log -soundhw pcspk


CRTI_OBJ=crti.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=crtn.o

OBJ_LINK_LIST:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)
INTERNAL_OBJS:=$(CRTI_OBJ) $(OBJ) $(CRTN_OBJ)

kernel: compile
	rm -f crepOS.img
	$(CC) -lgcc -no-pie -nodefaultlibs -nostartfiles -n -T link.ld -o Bin/crepOS.elf $(OBJ_LINK_LIST)
	dd if=/dev/zero bs=1M count=0 seek=64 of=crepOS.img
	parted -s crepOS.img mklabel msdos
	parted -s crepOS.img mkpart primary 1 100%
	echfs-utils -m -p0 crepOS.img quick-format 32768
	echfs-utils -m -p0 crepOS.img import qloader2.cfg qloader2.cfg
	echfs-utils -m -p0 crepOS.img import Bin/crepOS.elf crepOS.elf
	echfs-utils -m -p0 crepOS.img import Bin/crepOS.elf crepOS.img
	cd ../qloader2 && ./qloader2-install qloader2.bin ../crepOS/crepOS.img
	rm $(OBJ) crti.o crtn.o Bin/crepOS.elf
compile:
	$(CC) $(SCRC) $(CFLAGS) -c
	$(NASM) Kernel/boot.asm -o boot.o
	$(NASM) Kernel/exceptions.asm -o exceptions.o
	$(NASM) Kernel/interupts.asm -o interrupts.o
	$(AS) Kernel/crtn.s -o crtn.o
	$(AS) Kernel/crti.s -o crti.o

qemu: kernel
	touch serial.log
	qemu-system-x86_64 $(QEMUFLAGS) crepOS.img &
	tail -n0 -f serial.log

qemuinfo: kernel
	qemu-system-x86_64 -smp cpus=4 crepOS.img -m 4G -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware

qemudebug: kernel
	qemu-system-x86_64 -smp cpus=4 crepOS.img -m 4G -no-reboot -monitor stdio -d int -no-shutdown -vga vmware

clean:
	rm $(OBJ)
