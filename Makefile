BUILD_DIR := $(CURDIR)/build
LIB_DIR := $(CURDIR)/common/lib
LOG_DIR := $(CURDIR)/syms
ARCH = x86_64
OSNAME = ooos
TARGET_BARE = $(ARCH)-elf
TARGET_HOSTED = $(ARCH)-$(OSNAME)
READ := $(TARGET_BARE)-readelf
READFLAGS = -a -C -W
USE_GCC = 1
CORES = 2
INCLUDES := $(CURDIR)/common/include
INCLUDES_CXX := $(CURDIR)/common/include/c++
OVMF = /usr/share/OVMF
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
SUBDIRS = lib boot kernel test
OUT_IMG = $(OSNAME).img
EMULATE := qemu-system-$(ARCH)
EMUFLAGS := -rtc base=localtime -drive if=pflash,format=raw,unit=0,file=$(OVMF)/OVMF_CODE.fd,readonly=on -drive if=pflash,format=raw,unit=1,file=$(OVMF)/OVMF_VARS.fd,readonly=on\
	-cpu max -m 8G -M pc-q35-jammy-maxcpus,kernel-irqchip=split -device intel-iommu,intremap=on -net none -monitor vc -smp cores=$(CORES) -serial stdio
.PHONY: all $(SUBDIRS) asmtest
all: $(LOG_DIR) $(OUT_IMG) $(BUILD_DIR)
run: $(LOG_DIR) $(OUT_IMG)
	$(EMULATE) $(EMUFLAGS) -drive file=$(OUT_IMG),if=ide,format=raw
$(BUILD_DIR): 
	@mkdir -p $@
$(LOG_DIR):
	@mkdir -p $@
$(SUBDIRS):
	$(MAKE) -C $@
clean:
	rm -rf $(OUT_IMG) boot/uefi/*.o boot/uefi/*.a $(LOG_DIR) || true
	for dir in $(SUBDIRS); do \
		cd $$dir;\
		make clean ; \
		cd .. ;\
	done
$(OUT_IMG): create_image.sh $(SUBDIRS)
	sh $< $@ $(BUILD_DIR)
asmtest: 
	cd kernel && $(MAKE) asmtest
	cd lib && $(MAKE) asmtest