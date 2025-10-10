AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-gcc
ST := $(TARGET_HOSTED)-strip
READ = $(TARGET_HOSTED)-readelf
MODULE_OUTDIR = $(IMAGE_FILE_DIR)
CFLAGS = -O2 -fPIC -I$(SYSROOT)/usr/include -I$(PROJECT_DIR)/common/include/kernel -mno-red-zone -Wno-packed-bitfield-compat
CXXFLAGS := $(CFLAGS) -std=c++20
LDFLAGS := $(CFLAGS) L$(LIB_DIR) -nostartfiles -nodefaultlibs -lgcc -lstdc++ -lmodobj
PRE_OBJ_NAMES = modcrti.o modcrtbegin.o modcrt0.o
POST_OBJ_NAMES = modcrtend.o modcrtn.o
PRE_OBJS := $(addprefix $(LIB_DIR)/,$(PRE_OBJ_NAMES))
POST_OBJS := $(addprefix $(LIB_DIR)/,$(POST_OBJ_NAMES))