OUTDIR := $(LIB_DIR)
OBJ_BASE_DIR := $(CURDIR)/build
ASMT_DIR := $(CURDIR)/asm_test
CFLAGS := -ffreestanding -O2 -Wall -mno-red-zone $(addprefix -I,$(INCLUDES)) -Wno-packed-bitfield-compat 
CXXFLAGS := -DLIBCXXRT_WEAK_LOCKS -Wno-terminate $(CFLAGS)$(addprefix -I,$(INCLUDES_CXX)) -std=c++20 
CCFLAGS := $(CFLAGS) -std=gnu17
ARFLAGS := -rcs
AS := $(ARCH)-elf-as
CXX := $(ARCH)-elf-g++
AR := $(ARCH)-elf-ar
CC := $(ARCH)-elf-gcc
export OUTDIR
export OBJ_BASE_DIR
export CCFLAGS
export CXXFLAGS
export ARFLAGS
export AS
export AR
export CXX
export CC
export ASMT_DIR
SUBDIRS = libk libc
.PHONY: all $(SUBDIRS)
all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@
clean:
	rm -rf $(OBJ_BASE_DIR) || true
	rm -rf $(ASMT_DIR) || true
	rm -rf $(OUTDIR)/*.a $(OUTDIR)/*.o || true
asmtest:
	cd libk && $(MAKE) asmtest
	cd libc && $(MAKE) asmtest