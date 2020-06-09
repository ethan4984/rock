CXX = ~/opt/cross/bin/x86_64-elf-g++
AS = ~/opt/cross/bin/x86_64-elf-as
NASM = nasm -felf64

NORMAL_CXXFLAGS = -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -fno-exceptions -I.././
KERNEL_CRAP = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics
CXXFLAGS = $(NORMAL_CXXFLAGS) $(KERNEL_CRAP)

SCRCXX = $(shell find . -type f -name '*.cpp')
SRCA = $(shell find . -type f -name '*.asm')
SRCS = $(shell find . -type f -name '*.s')
OBJ = ../Bin/kernelMain.o ../Bin/ports.o ../Bin/videoIO.o ../Bin/boot.o ../Bin/stringUtils.o ../Bin/memoryUtils.o ../Bin/memoryParse.o ../Bin/physicalPageHandler.o ../Bin/pic.o ../Bin/scheduler.o ../Bin/acpiUtils.o ../Bin/memHandler.o ../Bin/madt.o ../Bin/pci.o ../Bin/vesa.o ../Bin/task.o ../Bin/thread.o ../Bin/keyboard.o ../Bin/mouse.o ../Bin/isr.o  ../Bin/assemblyISR.o ../Bin/prompt.o
OBJQ = kernelMain.o memoryParse.o physicalPageHandler.o pic.o scheduler.o acpiUtils.o memHandler.o madt.o pci.o vesa.o task.o thread.o keyboard.o mouse.o isr.o
QEMUFLAGS = -m 4G -vga vmware -serial file:serial.log -soundhw pcspk -machine q35


CRTI_OBJ=crti.o
CRTBEGIN_OBJ:=$(shell $(CXX) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CXX) $(CXXFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=crtn.o

OBJ_LINK_LIST:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)
INTERNAL_OBJS:=$(CRTI_OBJ) $(OBJ) $(CRTN_OBJ)

kernel: compile
	rm -f ../crepOS.img
	mv $(OBJQ) ../Bin
	$(CXX) -lgcc -no-pie -nodefaultlibs -nostartfiles -n -T ../link.ld -o ../Bin/crepOS.elf $(OBJ_LINK_LIST)
	dd if=/dev/zero bs=1M count=0 seek=64 of=../crepOS.img
	parted -s ../crepOS.img mklabel msdos
	parted -s ../crepOS.img mkpart primary 1 100%
	echfs-utils -m -p0 ../crepOS.img quick-format 32768
	echfs-utils -m -p0 ../crepOS.img import ../qloader2.cfg qloader2.cfg
	echfs-utils -m -p0 ../crepOS.img import ../Bin/crepOS.elf crepOS.elf
	echfs-utils -m -p0 ../crepOS.img import ../Bin/crepOS.elf ../crepOS.img
	cd ../qloader2 && ./qloader2-install qloader2.bin ../crepOS.img
	rm $(OBJ) crti.o crtn.o
compile:
	$(CXX) $(SCRCXX) $(CXXFLAGS) -c
	$(NASM) boot.asm -o ../Bin/boot.o
	$(NASM) int/isr.asm -o ../Bin/assemblyISR.o
	$(AS) crtn.s -o crtn.o
	$(AS) crti.s -o crti.o
	cd ../Slib && make all 
	cd ../Apps && make all

qemu: kernel
	touch serial.log
	qemu-system-x86_64 $(QEMUFLAGS) -hda ../crepOS.img -enable-kvm &
	tail -n0 -f serial.log

qemuinfo: kernel
	qemu-system-x86_64 -smp cpus=4 ../crepOS.img -m 4G -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware -enable-kvm

qemudebug: kernel
	qemu-system-x86_64 ../crepOS.img $(QEMUFLAGS) -no-reboot -monitor stdio -d int -no-shutdown

clean:
	rm $(OBJ)
