OUTDIR := $(LIB_DIR)
OBJ_BASE_DIR := $(CURDIR)/build
ASMT_DIR := $(CURDIR)/asm_test
CFLAGS := -ffreestanding -O2 -Wall -mno-red-zone $(addprefix -I,$(INCLUDES)) -Wno-packed-bitfield-compat 
CXXFLAGS := -DLIBCXXRT_WEAK_LOCKS $(CFLAGS)$(addprefix -I,$(INCLUDES_CXX)) -std=c++20
CCFLAGS := $(CFLAGS) -std=gnu23
ARFLAGS := -rcs
export OUTDIR
export OBJ_BASE_DIR
export CCFLAGS
export CXXFLAGS
export ARFLAGS
export TARGET_BARE
export TARGET_HOSTED
export ASMT_DIR
SUBDIRS = libk
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