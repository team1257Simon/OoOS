OBJ_BASE_DIR := $(CURDIR)/build
AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-g++
ST := $(TARGET_HOSTED)-strip
READ := $(TARGET_HOSTED)-readelf
OUTDIR := $(BUILD_DIR)/SYS/MOD
LDSCRIPT := $(CURDIR)/module.$(TARGET_HOSTED).lds
CFLAGS := -O2 -fPIC -fpie -I$(PROJECT_DIR)/common/include/kernel -I$(PROJECT_DIR)/common/include/c++ -Wno-packed-bitfield-compat -ffreestanding
CXXFLAGS := $(CFLAGS) -std=gnu++26 -fplugin=$(ATTR_PLUGIN) -nostdinc++ -freflection
CCFLAGS := $(CFLAGS) -std=gnu23 -fplugin=$(ATTR_PLUGIN)
LDFLAGS := -L$(LIB_DIR) -T $(LDSCRIPT) -pie -nostdlib -lgcc $(CFLAGS)
PRE_OBJ_NAMES := modcrti.o modcrtbegin.o modcrt0.o
POST_OBJ_NAMES := modcrtend.o modcrtn.o
PRE_OBJS := $(addprefix $(LIB_DIR)/,$(PRE_OBJ_NAMES))
POST_OBJS := $(addprefix $(LIB_DIR)/,$(POST_OBJ_NAMES))
ST_SYMS := __* _ZZ* _ZSt* _ZNSt* _ZN*St* _ZN*10__cxx* _ZN10__cxx* _ZN9pathscale* _ZT* _Znw* _Zna* _Zdl* _Zda* _ZGV* _ZL* _Unwind* _ITM* _impure_ptr fde_* uw_* execute_* seen_* unseen_* errno read_* frame_* linear_* search_* dwarf_reg_size_table get_cie_encoding pthread* str* abort free malloc calloc realloc mem* getenv setjmp longjmp register_tm_clones deregister_tm_clones *.*
ST_OP := -x -X -w $(addprefix --strip-symbol=, $(ST_SYMS))
export OUTDIR
export LDSCRIPT
export OBJ_BASE_DIR
export TARGET_BARE
export TARGET_HOSTED
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
export ST_OP
SUBDIRS = amd64_serial amd64_ahci e1000e
.PHONY: all $(SUBDIRS)
all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@
clean:
	rm -rf $(OBJ_BASE_DIR) || true
	rm -rf $(ASMT_DIR) || true
	for dir in $(SUBDIRS); do \
		cd $$dir; \
		$(MAKE) clean ; \
		cd .. ; \
	done
asmtest:
	cd amd64_serial && $(MAKE) asmtest
	cd amd64_ahci	&& $(MAKE) asmtest