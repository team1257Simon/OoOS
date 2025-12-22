AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
AR := $(TARGET_HOSTED)-ar
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-gcc
INCLUDES += $(PROJECT_DIR)/common/include/kernel
CFLAGS := -ffreestanding -O2 -Wall -mno-red-zone $(addprefix -I,$(INCLUDES)) -Wno-packed-bitfield-compat -mcrc32 -D__LIBK__
CXXFLAGS := -DLIBCXXRT_WEAK_LOCKS $(CFLAGS) $(addprefix -I,$(INCLUDES_CXX)) -std=gnu++26 -freflection -nostdinc++ -fplugin=$(ATTR_PLUGIN)
CCFLAGS := $(CFLAGS) -std=gnu23 -fplugin=$(ATTR_PLUGIN)
LDFLAGS := -r -flinker-output=rel -nostdlib
ARFLAGS := -rcs
OUT_A := $(OUTDIR)/libk.a
OBJDIR := $(OBJ_BASE_DIR)/libk
SUBDIRS = $(sort $(dir $(wildcard */*.*)))
OSUBDIRS = $(addprefix $(OBJDIR)/,$(SUBDIRS))
SSRC = $(wildcard *.S */*.S)
CSRC = $(wildcard *.c */*.c)
CPPSRC = $(wildcard *.cpp */*.cpp)
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.S=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
ASM_TEST = $(CPPSRC:%.cpp=%.test.asm) $(CSRC:%.c=%.test.asm)
SYM_LOG := $(LOG_DIR)/libk-syms.txt
.PHONY: all
all: $(OBJDIR) $(OSUBDIRS) $(OUT_A) $(SYM_LOG)
$(OBJDIR) $(OSUBDIRS):
	@mkdir -p $@
$(OUT_A): $(OBJECTS)
	@mkdir -p $(PROJECT_DIR)/common/include/asm-generated
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.S
	$(AS) -c $< -o $@
$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
$(SYM_LOG): $(OUT_A)
	$(READ) $(READFLAGS) $< > $@
asmtest: $(ASM_TEST)
clean:
	rm $(ASM_TEST) > /dev/null 2>/dev/null || true