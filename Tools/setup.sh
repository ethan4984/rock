#!/bin/sh

# build cross compiler

curl -O https://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz
curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz

tar xf gcc-9.2.0.tar.xz
tar xf binutils-2.34.tar.xz

export PREFIX="$HOME/opt/cross"
export TARGET="x86_64"
export PATH="$PREFIX/bin:$PATH"

mkdir build_binutils
mkdir build-gcc
mkdir ../Bin

cd build_binutils
../binutils-2.34/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd build-gcc
../gcc-*/configure --target=$TARGET --disable-nls --enable-languages=c,c++ --without-headers --prefix=$PREFIX
make all-gcc
make all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone' || true
make all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone'
make install-gcc
make install-target-libgcc

# build qloader2

git clone https://github.com/qword-os/echfs.git
cd echfs

make
sudo make install

cd ..

git clone https://github.com/qword-os/qloader2.git
cd qloader2/toolchain
./make_toolchain.sh

cd ..

make

cd ..
