#!/usr/bin/bash
BUILD_DIR="$PWD/build"
BINUTIL_VER="2.43"
GCC_VER="14.2.0"
GDB_VER="15.1"

TARGET="x86_64-elf"
PREFIX="${BUILD_DIR}/cc"

BIN_D="${PREFIX}/bin"

PATH=$PATH:$BIN_D

