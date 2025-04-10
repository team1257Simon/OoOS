LIB_NAME = libc
FILE_EXTENSION = so
NEWLIB_VERSION_MAJOR = 3
NEWLIB_VERSION_MINOR = 0.0
AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-gcc
ST := $(TARGET_HOSTED)-strip
READ = $(TARGET_HOSTED)-readelf
BASE_NAME = $(LIB_NAME).$(FILE_EXTENSION)
SO_NAME = $(BASE_NAME).$(NEWLIB_VERSION_MAJOR)
FULL_NAME = $(SO_NAME).$(NEWLIB_VERSION_MINOR)
CFLAGS = -O2 -fPIC -I$(SYSROOT)/usr/include
CCFLAGS = $(CFLAGS) -std=gnu23 -w
CXXFLAGS = $(CFLAGS) -std=c++20 -w
LDFLAGS = $(CFLAGS) -nostartfiles -nodefaultlibs -Wl,-Bsymbolic -Wl,-soname,$(SO_NAME) -shared -lgcc
OUTDIR = $(IMAGE_FILE_DIR)/lib
OBJDIR := $(OBJ_BASE_DIR)/libc
SUBDIRS = $(sort $(dir $(wildcard */*.*))) sys/ooos machine/x86_64
OSUBDIRS = $(addprefix $(OBJDIR)/,$(SUBDIRS))
ASUBDIRS = $(addprefix $(ASMT_DIR)/,$(SUBDIRS))
SSRC = $(wildcard *.S */*.S machine/x86_64/*.S)
CSRC = $(wildcard *.c */*.c */*/*.c)
CPPSRC = $(wildcard *.cpp */*.cpp)
ARCH_OBJECT_NAMES = $(SSRC:%.S=%.o)
ARCH_OBJECTS = $(addprefix $(OBJDIR)/,$(ARCH_OBJECT_NAMES))
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) arch-libc.o
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES)) 
ASM_TEST = $(addprefix $(ASMT_DIR)/, $(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/, $(CSRC:%.c=%.test.asm))
OUT_SO = $(OUTDIR)/$(BASE_NAME)
SYM_LOG := $(LOG_DIR)/libc-syms.txt
CRT0 = $(OUTDIR)/crt0.o
CRTI = $(OUTDIR)/crti.o
CRTN = $(OUTDIR)/crtn.o
MATH_LIB = $(OUTDIR)/libm.a
.PHONY: all
all: $(OBJDIR) $(OSUBDIRS) $(OUTDIR) $(OUT_SO) $(CRT0) $(CRTI) $(CRTN) $(SYM_LOG)
$(OUTDIR) $(OBJDIR) $(OSUBDIRS) $(ASMT_DIR) $(LOG_DIR):
	@mkdir -p $@
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/arch-libc.o: $(ARCH_OBJECTS)
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
$(CRT0): sys/ooos/crt0_dyn.S
	$(AS) -fPIC -c $< -o $@
$(CRTI): sys/ooos/crti.s
	$(AS) -fPIC -c $< -o $@
$(CRTN): sys/ooos/crtn.s
	$(AS) -fPIC -c $< -o $@
$(OUT_SO): $(OBJECTS)
	$(LD) $(OBJECTS) $(MATH_LIB) $(LDFLAGS) -o $@
	$(ST) -s $@
asmtest: $(ASMT_DIR) $(ASM_TEST)
