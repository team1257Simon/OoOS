AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-gcc
ST := $(TARGET_HOSTED)-strip
READ = $(TARGET_HOSTED)-readelf
VERSION_MAJOR = 0
VERSION_MINOR = 0.0
FILE_EXTENSION = so
BASE_NAME = ld-ooos.$(FILE_EXTENSION)
SO_NAME = $(BASE_NAME)
FULL_NAME = $(SO_NAME).$(VERSION_MAJOR).$(VERSION_MINOR)
CFLAGS = -O2 -fPIC 
CCFLAGS = $(CFLAGS) -std=gnu23 -Wall
CXXFLAGS = $(CFLAGS) -std=c++20 -Wall -fno-exceptions -fcheck-new -nostdinc++
LDFLAGS = $(CFLAGS) -L$(SYS_LIB) -L$(LIB_GCC) -L$(SYSROOT)/usr/x86_64-ooos/lib -nostartfiles -nodefaultlibs -Wl,-Bsymbolic -Wl,-soname,$(SO_NAME) -shared
OUTDIR = $(IMAGE_FILE_DIR)/lib
OBJDIR := $(OBJ_BASE_DIR)/ld-ooos
SSRC = $(wildcard *.S */*.S)
CSRC = $(wildcard *.c */*.c)
CPPSRC = $(wildcard *.cpp */*.cpp)
ARCH_OBJECT_NAMES = $(SSRC:%.S=%.o)
ARCH_OBJECTS = $(addprefix $(OBJDIR)/,$(ARCH_OBJECT_NAMES))
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) arch-ldso.o
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES)) 
ASM_TEST = $(addprefix $(ASMT_DIR)/, $(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/, $(CSRC:%.c=%.test.asm))
OUT_SO = $(OUTDIR)/$(BASE_NAME)
SYM_LOG := $(LOG_DIR)/ld-ooos-syms.txt
.PHONY: all
all: $(OBJDIR) $(OUTDIR) $(OUT_SO) $(SYM_LOG)
$(OUTDIR) $(OBJDIR) $(ASMT_DIR) $(LOG_DIR):
	@mkdir -p $@
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/arch-ldso.o: $(ARCH_OBJECTS)
	$(LD) -r -o $@ $(ARCH_OBJECTS)
$(OBJDIR)/%.o: %.S
	$(AS) -fPIC -c $< -o $@
$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(ASMT_DIR)/%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
$(ASMT_DIR)/%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
$(SYM_LOG): $(OUT_SO) $(LOG_DIR) 
	$(READ) $(READFLAGS) $< > $@
$(OUT_SO): $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@
	$(ST) -s $@
asmtest: $(ASMT_DIR) $(ASM_TEST)
