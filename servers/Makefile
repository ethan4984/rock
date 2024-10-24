build_servers:
	cd sched && make && cp sched ../../build/system-root/servers/sched
	cd fs/vfs && make && cp vfs ../../../build/system-root/servers/vfs
	cd io/nvme && make && cp nvme ../../../build/system-root/servers/nvme

clean:
	cd sched && make clean
	cd fs/vfs && make clean
	cd io/nvme && make clean
