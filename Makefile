CXX = ~/opt/cross/bin/x86_64-elf-g++
NASM = nasm -felf64

NORMAL_CXXFLAGS = -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -fno-exceptions -I.  -Wno-unused-parameter
KERNEL_CXXFLAGS = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics -std=c++17

CXXFLAGS = $(NORMAL_CXXFLAGS) $(KERNEL_CXXFLAGS)

CXX_SRC = $(shell find . -type f -name '*.cpp')

QEMUFLAGS = -m 4G -vga vmware -serial file:serial.log -soundhw pcspk -machine q35 -smp 4

CRTI_OBJ=kernel/crti.o
CRTBEGIN_OBJ:=$(shell $(CXX) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CXX) $(CXXFLAGS) -print-file-name=crtend.o)
CRTN_OBJ=kernel/crtn.o

OBJ_LINK_LIST:=$(CRTI_OBJ) $(CRTBEGIN_OBJ) Bin/*.o $(CRTEND_OBJ) $(CRTN_OBJ)
INTERNAL_OBJS:=$(CRTI_OBJ) $(OBJ) $(CRTN_OBJ)

build:
	rm -f rock.img
	$(CXX) $(CXXFLAGS) $(CXX_SRC) -c
	mv *.o Bin
	$(NASM) kernel/boot.asm -o Bin/boot.o
	$(NASM) kernel/crtn.asm -o kernel/crtn.o
	$(NASM) kernel/crti.asm -o kernel/crti.o
	$(NASM) kernel/int/isr.asm -o Bin/isr.o
	$(NASM) kernel/int/gdt.asm -o Bin/gdtAsm.o
	$(CXX) -lgcc -no-pie -nodefaultlibs -nostartfiles -n -T linker.ld -o Bin/rock.elf $(OBJ_LINK_LIST)
	dd if=/dev/zero bs=1M count=0 seek=64 of=rock.img
	parted -s rock.img mklabel msdos
	parted -s rock.img mkpart primary 1 100%
	echfs-utils -m -p0 rock.img quick-format 32768
	echfs-utils -m -p0 rock.img import kernel/qloader2.cfg qloader2.cfg
	echfs-utils -m -p0 rock.img import Bin/rock.elf rock.elf
	echfs-utils -m -p0 rock.img import Bin/rock.elf rock.img
	cd qloader2 && ./qloader2-install qloader2.bin ../rock.img
	rm kernel/*.o

qemu: build
	touch serial.log
	qemu-system-x86_64 $(QEMUFLAGS) -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 -enable-kvm &
	tail -n0 -f serial.log

info: build
	qemu-system-x86_64 -smp cpus=4 rock.img -m 4G -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware 

debug: build
	qemu-system-x86_64 rock.img $(QEMUFLAGS) -no-reboot -monitor stdio -d int -no-shutdown
