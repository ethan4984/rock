CXX = ~/opt/cross/bin/x86_64-elf-g++
NASM = nasm -felf64

NORMAL_CXXFLAGS = -ffreestanding -Wall -O2 -Wextra -fno-stack-protector -fno-exceptions -I.  -Wno-unused-parameter
KERNEL_CXXFLAGS = -fno-pic -mno-sse -mno-sse2 -mno-mmx -mno-80387 -mno-red-zone -gdwarf -mcmodel=kernel -fno-omit-frame-pointer -fno-threadsafe-statics -std=c++17

CXXFLAGS = $(NORMAL_CXXFLAGS) $(KERNEL_CXXFLAGS)

CXX_SRC = $(shell find . -type f -name '*.cpp')

QEMUFLAGS = -m 4G -vga vmware -serial file:serial.log -smp 4 -netdev user,id=n1 -device e1000,netdev=n1 

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
	$(NASM) kernel/sched/scheduler.asm -o Bin/schedulerASM.o
	$(NASM) userspace/program.asm -o Bin/userspace.o
	nasm -fbin kernel/sched/smp.asm -o Bin/smpASM.bin
	$(NASM) kernel/real.asm -o Bin/real.o
	$(CXX) -lgcc -no-pie -nodefaultlibs -nostartfiles -n -T linker.ld -o Bin/rock.elf $(OBJ_LINK_LIST)
	dd if=/dev/zero bs=1M count=0 seek=64 of=rock.img
	parted -s rock.img mklabel msdos
	parted -s rock.img mkpart primary 1 100%
	rm -rf diskImage/
	mkdir diskImage
	sudo losetup -Pf --show rock.img > loopback_dev
	sudo partprobe `cat loopback_dev`
	sudo mkfs.ext2 `cat loopback_dev`p1
	sudo mount `cat loopback_dev`p1 diskImage
	sudo mkdir diskImage/boot
	sudo cp Bin/rock.elf diskImage/boot/
	sudo cp kernel/limine.cfg diskImage/
	sync
	sudo umount diskImage/
	sudo losetup -d `cat loopback_dev`
	rm -rf diskImage loopback_dev
	cd limine && ./limine-install ../rock.img
	rm kernel/*.o

qemu: build
	touch serial.log
	qemu-system-x86_64 $(QEMUFLAGS) -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -enable-kvm &
	tail -n0 -f serial.log

info: build
	qemu-system-x86_64 -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware -enable-kvm

debug: build
	qemu-system-x86_64 -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -no-shutdown
