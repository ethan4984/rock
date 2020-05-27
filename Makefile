default:
	cd Kernel && make

qemu:
	cd Kernel && make qemu

qemuinfo:
	cd Kernel && make qemuinfo

qemudebug:
	cd Kernel && make qemudebug
