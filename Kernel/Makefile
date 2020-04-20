CC = ~/opt64/cross/bin/x86_64-elf-g++
AS = ~/opt64/cross/bin/x86_64-elf-as
NASM = nasm -felf64

NORMAL_CFLAGS = -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -fno-exceptions -I../Kernel -I../libc++
KERNEL_CRAP = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics
CFLAGS = $(NORMAL_CFLAGS) $(KERNEL_CRAP)

SCRC = $(shell find . -type f -name '*.cpp')
SRCA = $(shell find . -type f -name '*.asm')
SRCS = $(shell find . -type f -name '*.s')
OBJ = ../Bin/alloc.o ../Bin/interrupt.o ../Bin/memory.o ../Bin/keyboard.o ../Bin/kernel_init.o ../Bin/paging.o ../Bin/port.o ../Bin/process.o ../Bin/scheduler.o ../Bin/shell.o ../Bin/shitio.o ../Bin/boot.o ../Bin/exceptions.o ../Bin/interrupts.o ../Bin/string.o ../Bin/sound.o ../Bin/acpi.o ../Bin/vesa.o ../Bin/mouse.o ../Bin/pci.o ../Bin/ahci.o
OBJQ = alloc.o interrupt.o memory.o keyboard.o kernel_init.o paging.o port.o process.o scheduler.o shell.o shitio.o sound.o acpi.o vesa.o mouse.o pci.o ahci.o
QEMUFLAGS = -smp cpus=4 -m 4G -vga vmware -serial file:serial.log -soundhw pcspk -enable-kvm


CRTI_OBJ=crti.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=crtn.o

OBJ_LINK_LIST:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)
INTERNAL_OBJS:=$(CRTI_OBJ) $(OBJ) $(CRTN_OBJ)

kernel: compile
	rm -f ../crepOS.img
	mv $(OBJQ) ../Bin
	$(CC) -lgcc -no-pie -nodefaultlibs -nostartfiles -n -T ../link.ld -o ../Bin/crepOS.elf $(OBJ_LINK_LIST)
	dd if=/dev/zero bs=1M count=0 seek=64 of=../crepOS.img
	parted -s ../crepOS.img mklabel msdos
	parted -s ../crepOS.img mkpart primary 1 100%
	echfs-utils -m -p0 ../crepOS.img quick-format 32768
	echfs-utils -m -p0 ../crepOS.img import ../qloader2.cfg qloader2.cfg
	echfs-utils -m -p0 ../crepOS.img import ../Bin/crepOS.elf crepOS.elf
	echfs-utils -m -p0 ../crepOS.img import ../Bin/crepOS.elf ../crepOS.img
	cd ../qloader2 && ./qloader2-install qloader2.bin ../crepOS.img
	rm $(OBJ) crti.o crtn.o ../Bin/crepOS.elf
compile:
	$(CC) $(SCRC) $(CFLAGS) -c
	$(NASM) boot.asm -o ../Bin/boot.o
	$(NASM) exceptions.asm -o ../Bin/exceptions.o
	$(NASM) interupts.asm -o ../Bin/interrupts.o
	$(AS) crtn.s -o crtn.o
	$(AS) crti.s -o crti.o

qemu: kernel
	touch serial.log
	qemu-system-x86_64 $(QEMUFLAGS) ../crepOS.img &
	tail -n0 -f serial.log

qemuinfo: kernel
	qemu-system-x86_64 -smp cpus=4 ../crepOS.img -m 4G -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware

qemudebug: kernel
	qemu-system-x86_64 -smp cpus=4 ../crepOS.img -m 4G -no-reboot -monitor stdio -d int -no-shutdown -vga vmware

clean:
	rm $(OBJ)
