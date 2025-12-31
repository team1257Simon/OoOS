PROJECT_DIR := $(CURDIR)
BUILD_DIR := $(CURDIR)/build
LIB_DIR := $(CURDIR)/common/lib
LOG_DIR := $(CURDIR)/syms
ARCH = x86_64
OSNAME = ooos
TARGET_BARE = $(ARCH)-elf
TARGET_HOSTED = $(ARCH)-$(OSNAME)
IMAGE_FILE_DIR = $(CURDIR)/img
READ := $(TARGET_BARE)-readelf
READFLAGS = -a -C -W
USE_GCC = 1
CORES = 2
INCLUDES := $(CURDIR)/common/include
INCLUDES_CXX := $(CURDIR)/common/include/c++
OVMF = /usr/share/OVMF
SYSROOT = /usr/local/ooos_sysroot
SYS_LIB = $(SYSROOT)/usr/lib
LIB_GCC = $(SYS_LIB)/gcc/x86_64-ooos/14.2.0
ATTR_PLUGIN := $(LIB_DIR)/ooos_attrs.so
export PROJECT_DIR
export ARCH
export OSNAME
export TARGET_BARE
export TARGET_HOSTED
export READ
export READFLAGS
export USE_GCC
export INCLUDES
export INCLUDES_CXX
export BUILD_DIR
export LIB_DIR
export LOG_DIR
export SYSROOT
export SYS_LIB
export LIB_GCC
export IMAGE_FILE_DIR
export ATTR_PLUGIN
SUBDIRS = headergen lib modules boot kernel test
OUT_IMG = $(OSNAME).img
EMULATE := qemu-system-$(ARCH)
EMUFLAGS := -rtc base=utc -device nec-usb-xhci														\
	-drive if=pflash,format=raw,unit=0,file=$(OVMF)/OVMF_CODE.fd,readonly=on						\
	-drive if=pflash,format=raw,unit=1,file=$(OVMF)/OVMF_VARS.fd,readonly=on						\
	-cpu max,+sse2,+sse4.1,+sse4.2,+sse4a,+ssse3,+xsave												\
	-m 8G -M pc-q35-jammy-maxcpus,kernel-irqchip=split -smp cores=$(CORES)							\
	-device intel-iommu,intremap=on																	\
	-netdev user,id=net0 -device e1000e,netdev=net0													\
	-object filter-dump,id=f1,netdev=net0,file=dump.bin												\
	-monitor vc -serial stdio
.PHONY: all $(SUBDIRS) asmtest asmtest_kernel clean distclean
all: $(LOG_DIR) $(OUT_IMG)
run: $(LOG_DIR) $(OUT_IMG)
	$(EMULATE) $(EMUFLAGS) -drive file=$(OUT_IMG),if=ide,format=raw
$(BUILD_DIR):
	@mkdir -p $@
$(LOG_DIR):
	@mkdir -p $@
$(SUBDIRS):
	$(MAKE) -C $@
$(ATTR_PLUGIN):
	$(MAKE) -C lib/ooos-attrs
clean:
	rm -rf $(OUT_IMG) *.bin common/include/asm-generated/* boot/*.o boot/uefi/*.o boot/uefi/*.a $(LOG_DIR) || true
	cd kernel && $(MAKE) clean
	cd headergen && $(MAKE) clean
	cd test && $(MAKE) clean
distclean:
	rm -rf $(OUT_IMG) $(ATTR_PLUGIN) boot/*.o boot/uefi/*.o boot/uefi/*.a $(LOG_DIR) || true
	rm *.bin || true
	rm -rf common/include/asm-generated/* || true
	for dir in $(SUBDIRS); do \
		cd $$dir; \
		$(MAKE) clean ; \
		cd .. ; \
	done
$(OUT_IMG): $(ATTR_PLUGIN) $(BUILD_DIR) create_image.sh $(SUBDIRS)
	sh create_image.sh $@ $(BUILD_DIR) $(IMAGE_FILE_DIR)
asmtest: $(ATTR_PLUGIN)
	cd kernel && $(MAKE) asmtest
	cd modules && $(MAKE) asmtest
	cd lib && $(MAKE) asmtest
asmtest_kernel: $(ATTR_PLUGIN)
	cd kernel && $(MAKE) asmtest