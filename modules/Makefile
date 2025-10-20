OBJ_BASE_DIR := $(CURDIR)/build
ASMT_DIR := $(CURDIR)/asm_test
AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-g++
ST := $(TARGET_HOSTED)-strip
READ = $(TARGET_HOSTED)-readelf
OUTDIR = $(IMAGE_FILE_DIR)/sys
LDSCRIPT = $(CURDIR)/module.$(TARGET_HOSTED).lds
CFLAGS := -O2 -fPIC -fpie -I$(PROJECT_DIR)/common/include/kernel -I$(PROJECT_DIR)/common/include -Wno-packed-bitfield-compat -ffreestanding
CXXFLAGS := $(CFLAGS) -std=gnu++26
CCFLAGS := $(CFLAGS) -std=gnu23
LDFLAGS := -L$(LIB_DIR) -T $(LDSCRIPT) -pie -nostdlib -lgcc -lstdc++ $(CFLAGS)
PRE_OBJ_NAMES = modcrti.o modcrtbegin.o modcrt0.o
POST_OBJ_NAMES = modcrtend.o modcrtn.o
PRE_OBJS := $(addprefix $(LIB_DIR)/,$(PRE_OBJ_NAMES))
POST_OBJS := $(addprefix $(LIB_DIR)/,$(POST_OBJ_NAMES))
export OUTDIR
export LDSCRIPT
export OBJ_BASE_DIR
export TARGET_BARE
export TARGET_HOSTED
export ASMT_DIR
export AS
export CXX
export CC
export LD
export ST
export READ
export CFLAGS
export CXXFLAGS
export CCFLAGS
export LDFLAGS
export PRE_OBJ_NAMES
export POST_OBJ_NAMES
export PRE_OBJS
export POST_OBJS
SUBDIRS = test_module amd64_serial
.PHONY: all $(SUBDIRS)
all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@
clean:
	rm -rf $(OBJ_BASE_DIR) || true
	rm -rf $(ASMT_DIR) || true
	rm -rf $(OUTDIR)/*.a $(OUTDIR)/*.o || true
	rm -rf $(IMAGE_FILE_DIR)/lib/*.* || true
	for dir in $(SUBDIRS); do \
		cd $$dir; \
		$(MAKE) clean ; \
		cd .. ; \
	done
asmtest:
	cd test_module && $(MAKE) asmtest
	cd amd64_serial && $(MAKE) asmtest