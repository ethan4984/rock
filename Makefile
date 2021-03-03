QEMUFLAGS = -m 4G -vga vmware -serial file:serial.log -smp 4

build:
	cd user && make main
	cd kernel && make build -j$(nproc)
	rm -f rock.img
	dd if=/dev/zero bs=1M count=0 seek=64 of=rock.img
	parted -s rock.img mklabel msdos
	parted -s rock.img mkpart primary 1 100%
	rm -rf disk_image/
	mkdir disk_image
	sudo losetup -Pf --show rock.img > loopback_dev
	sudo mkfs.ext2 `cat loopback_dev`p1
	sudo mount `cat loopback_dev`p1 disk_image
	sudo mkdir disk_image/boot
	sudo cp kernel/bin/rock.elf disk_image/boot/
	sudo cp kernel/limine.cfg disk_image/
	sudo cp user/test disk_image/
	sudo cp -r user/build/system-root/. disk_image
	sync
	sudo umount disk_image/
	sudo losetup -d `cat loopback_dev`
	rm -rf disk_image loopback_dev
	limine-install rock.img 

qemu: build
	touch serial.log
	qemu-system-x86_64 $(QEMUFLAGS) -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -enable-kvm &
	tail -n0 -f serial.log

rebuild_mlibc:
	cd build && xbstrap install mlibc --rebuild

info: build
	qemu-system-x86_64 -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware -enable-kvm

debug: build
	qemu-system-x86_64 -drive id=disk,file=rock.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -no-shutdown

rebuild_mlibc:
	cd user/build && xbstrap install mlibc --rebuild
