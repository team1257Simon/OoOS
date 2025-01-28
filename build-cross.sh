#!/bin/sh
while getopts t:p: flag
do
	case "${flag}" in
		t) TARGET=${OPTARG} ;;
		p) PREFIX=${PREFIX} ;;
	esac
done
echo "Building with target: ${TARGET:=x86_64-elf} and prefix: ${PREFIX:=/usr/local}"
export PATH="$PREFIX/bin:$PATH"
mkdir -p ~/src
cd ~/src
mkdir -p build-binutils
mkdir -p build-gcc
mkdir -p build-gdb
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libisl-dev
sudo usermod -a -G kvm ${USER}
wget https://ftp.gnu.org/gnu/binutils/binutils-2.43.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.gz
wget https://ftp.gnu.org/gnu/gdb/gdb-15.1.tar.gz
tar -xf binutils-*.tar.gz
tar -xf gcc-*.tar.gz
tar -xf gdb-*.tar.gz
rm *.gz
cd build-binutils
../binutils-*/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
make
sudo make install
cd ../build-gdb
../gdb-*/configure --target=$TARGET --prefix=$PREFIX --disable-werror
make all-gdb
sudo make install-gdb
cd ..
echo "MULTILIB_OPTIONS += mno-red-zone 
MULTILIB_DIRNAMES += no-red-zone" > gcc-*/gcc/config/i386/t-x86_64-elf
cd build-gcc
../gcc-*/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
sudo make install-gcc install-target-libgcc
cd ~