#!/bin/sh
OOOS_DIR = `pwd`
BUILD_DIR = ${BUILD_DIR := ~/src}
SYSROOT = ${SYSROOT := ~/ooos_sysroot}
PREFIX_BASE = ${PREFIX_BASE := usr}
PREFIX = "$SYSROOT/$PREFIX_BASE"
BARE_CC_PREFIX = ${BARE_CC_PREFIX = /usr/local}
AUTOTOOLS_DIR = "$BUILD_DIR/autotools"
GCC_AM = $AUTOTOOLS_DIR/am115/bin/automake
GCC_AC = $AUTOTOOLS_DIR/ac269/autoconf
NL_AC = $AUTOTOOLS_DIR/ac265/autoconf
NL_RC = $AUTOTOOLS_DIR/ac265/autoreconf
mkdir -p $BUILD_DIR
mkdir $AUTOTOOLS_DIR
cd $AUTOTOOLS_DIR
wget https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz
wget https://ftp.gnu.org/gnu/automake/automake-1.11.tar.gz
wget https://ftp.gnu.org/gnu/autoconf/autoconf-2.65.tar.gz
wget https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
tar -xzf automake-1.15.1.tar.gz
tar -xzf automake-1.11.tar.gz
tar -xzf autoconf-2.65.tar.gz
tar -xzf autoconf-2.69.tar.gz
mkdir am115
mkdir am111
mkdir ac265
mkdir ac269
cd automake-1.15.1
./configure --prefix=$AUTOTOOLS_DIR/am115
make && make install
cd ../automake-1.11
./configure --prefix=$AUTOTOOLS_DIR/am111
make && make install
cd ../autoconf-2.65
./configure --prefix=$AUTOTOOLS_DIR/ac265
make && make install
cd ../autoconf-2.69
./configure --prefix=$AUTOTOOLS_DIR/ac269
make && make install
cd $BUILD_DIR
mkdir build-binutils
mkdir build-gcc
mkdir build-newlib
git clone https://sourceware.org/git/binutils-gdb.git
cd binutils-gdb
git checkout binutils-2_43_1
git apply $OOOS_DIR/lib/patches/binutils-gdb-2.43.1.diff
cp -lRf $OOOS_DIR/lib/binutils-gdb $BUILD_DIR
cd ld
$GCC_AM
cd $BUILD_DIR/build-binutils
../binutils-gdb/configure --target=x86_64-ooos --with-sysroot=$SYSROOT --prefix=$PREFIX --disable-nls --disable-werror --enable-shared
make && make install
git clone https://github.com/reswitched/newlib.git
cd newlib
git apply $OOOS_DIR/lib/patches/newlib-3.0.0.diff
cp -lRf $OOOS_DIR/lib/newlib/libc/sys/ooos $BUILD_DIR/newlib/newlib/libc/sys
cd $BUILD_DIR/newlib/newlib/libc/sys
$NL_RC -I../../ -I../
cd $BUILD_DIR/newlib/newlib/libc/machine/x86_64
$NL_RC -I../../../ -I../../ -I../ || true # autoheader will fail, but the configure still works so we don't want that to kill the script. TODO: figure out something less hacky
cd $BUILD_DIR/build-newlib
../newlib/configure --target=x86_64-ooos --prefix=$PREFIX --with-sysroot=$SYSROOT --enable-shared --enable-host-shared
make && make install
cd $BUILD_DIR
git clone https://gcc.gnu.org/git/gcc.git
cd gcc
git checkout releases/gcc-14.2.0
git apply $OOOS_DIR/lib/patches/gcc-14.2.diff
cp -lRf $OOOS_DIR/lib/gcc $BUILD_DIR/gcc
cd libstdc++-v3
$GCC_AC
cd $BUILD_DIR/build-gcc
../gcc/configure --target=x86_64-ooos --enable-languages=c,c++ --with-sysroot=$SYSROOT --prefix=$PREFIX --with-newlib --enable-shared
make all-gcc all-target-libgcc
make install-gcc install-target-libgcc
make all-target-libstdc++-v3
make install-target-libstdc++-v3