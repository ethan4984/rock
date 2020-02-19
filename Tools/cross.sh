#!/bin/sh

echo c,c++ or c, or c++?
read lang

cores=grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}'

curl -O https://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz
curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz

tar xf gcc-9.2.0.tar.xz
tar xf binutils-2.34.tar.xz

export PREFIX="$HOME/opt/cross"
export TARGET="i686-elf"
export PATH="$PREFIX/bin:$PATH"

mkdir build_binutils
mkdir build-gcc

cd build_binutils
../binutils-2.34/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(cores)
make install -j$(cores)

if[-z"$(which -- $TARGET-as)"] then
	echo fatal: Error in building binutils
	exit 1
fi

cd ../build-gcc
../gcc-9.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=$(lang) --without-headers
make all-gcc -j$(cores)
make all-target=libgcc -j$(cores)
make install-gcc -j$(cores)
make install-target-libgcc -j$(cores)

if[-e"$($HOME/opt/cross/build-gcc/i686-elf-gcc)" ] then
	echo fatal: Error in building gcc
	exit 1
fi






