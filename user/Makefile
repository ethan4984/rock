CC = build/tools/host-gcc/bin/x86_64-rock-gcc

main:
	$(CC) test.c -o test

build_toolchain:
	mkdir -p build
	cd build && xbstrap init .. && xbstrap install --all && xbstrap install-tool host-gcc
