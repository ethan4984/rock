default:
	cd libc++ && make
	cd Kernel && make

qemu:
	cd libc++ && make
	cd Kernel && make qemu

qemuinfo:
	cd libc++ && make
	cd Kernel && make qemuinfo

qemudebug:
	cd libc++ && make
	cd Kernel && make qemudebug
