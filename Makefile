QEMUFLAGS = -m 4G -vga vmware -serial file:serial.log -smp 4 -netdev user,id=n1 -device e1000,netdev=n1 

build:
	cd user && make
	cd kernel && make build -j$(nproc)
	rm -f rock.img
	dd if=/dev/zero bs=1M count=0 seek=64 of=rock.img
	parted -s rock.img mklabel msdos
	parted -s rock.img mkpart primary 1 100%
	rm -rf diskImage/
	mkdir diskImage
	sudo losetup -Pf --show rock.img > loopback_dev
	sudo mkfs.ext2 `cat loopback_dev`p1
	sudo mount `cat loopback_dev`p1 diskImage
	sudo mkdir diskImage/boot
	sudo cp kernel/bin/rock.elf diskImage/boot/
	sudo cp kernel/limine.cfg diskImage/
	sudo cp user/test.elf diskImage/
	sync
	sudo umount diskImage/
	sudo losetup -d `cat loopback_dev`
	rm -rf diskImage loopback_dev
	limine-install rock.img 

qemu: build
	touch serial.log
	qemu-system-x86_64 $(QEMUFLAGS) -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -enable-kvm &
	tail -n0 -f serial.log

info: build
	qemu-system-x86_64 -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware 

debug: build
	qemu-system-x86_64 -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -no-shutdown
