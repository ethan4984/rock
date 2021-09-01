QEMUFLAGS = -m 4G \
			-serial file:serial.log \
			-smp 4 \
			-drive file=disk.img,if=none,id=NVME1 \
			-device nvme,drive=NVME1,serial=nvme \
			-drive id=disk,file=rock.img,if=none \
			-device ahci,id=ahci \
			-device ide-hd,drive=disk,bus=ahci.0 \
			-drive if=none,id=usbstick,file=usb.img \
			-usb \
			-device qemu-xhci \
			-device intel-hda,debug=0 -device hda-duplex \
			-trace hda* \
	 		-trace pci_nvme*

build:
	cd kernel && make -j16
	cd user && make 
	rm -f rock.img disk.img usb.img
	dd if=/dev/zero bs=1M count=0 seek=512 of=rock.img
	dd if=/dev/zero bs=1M count=0 seek=512 of=disk.img
	dd if=/dev/zero bs=1M count=0 seek=512 of=usb.img
	parted -s rock.img mklabel msdos
	parted -s rock.img mkpart primary 1 100%
	rm -rf disk_image/
	mkdir disk_image
	sudo losetup -Pf --show rock.img > loopback_dev
	sudo mkfs.ext2 `cat loopback_dev`p1
	sudo mount `cat loopback_dev`p1 disk_image
	sudo mkdir disk_image/boot
	sudo cp kernel/rock.elf disk_image/boot/
	sudo cp kernel/limine.cfg disk_image/
	sudo cp tools/limine/limine.sys disk_image/boot/
	sudo cp -r user/build/system-root/. disk_image/.
	sudo cp user/.bashrc disk_image/
	sudo cp user/test.asm disk_image/
	sudo cp user/test.c disk_image/
	sync
	sudo umount disk_image/
	sudo losetup -d `cat loopback_dev`
	rm -rf disk_image loopback_dev
	tools/limine/limine-install-linux-x86_64 rock.img 
	parted -s disk.img mklabel gpt
	parted -s disk.img mkpart primary 2048s 100%
	sudo losetup -Pf --show disk.img > loopback_dev
	sudo mkfs.ext2 `cat loopback_dev`p1
	rm -rf disk_image/
	mkdir disk_image
	sudo mount `cat loopback_dev`p1 disk_image
	sync
	sudo umount disk_image/
	sudo losetup -d `cat loopback_dev`
	sync
	rm -rf loopback_dev

rebuild_mlibc:
	cd user/build && xbstrap install mlibc --rebuild

qemu: build
	touch serial.log
	qemu-system-x86_64 $(QEMUFLAGS) &
	tail -n0 -f serial.log

info: build
	qemu-system-x86_64 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -D qemu.log -no-shutdown -vga vmware

debug: build
	qemu-system-x86_64 $(QEMUFLAGS) -no-reboot -monitor stdio -d int -no-shutdown
