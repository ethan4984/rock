#!/bin/sh

curl -O https://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz
curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz

tar xf gcc-9.2.0.tar.xz
tar xf binutils-2.34.tar.xz

export PREFIX="$HOME/opt/cross"
export TARGET="i686-elf"
export PATH="$PREFIX/bin:$PATH"

mkdir build_binutils
mkdir build-gcc
mkdir ../Bin

cd build_binutils
../binutils-2.34/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd ../build-gcc
../gcc-9.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target=libgcc
make install-gcc
make install-target-libgcc






