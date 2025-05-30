AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
AR := $(TARGET_HOSTED)-ar
CC := $(TARGET_HOSTED)-gcc
CFLAGS := -ffreestanding -O2 -Wall -mno-red-zone $(addprefix -I,$(INCLUDES)) -Wno-packed-bitfield-compat -mcrc32
CXXFLAGS := -DLIBCXXRT_WEAK_LOCKS $(CFLAGS) $(addprefix -I,$(INCLUDES_CXX)) -std=gnu++23 -nostdinc++
CCFLAGS := $(CFLAGS) -std=gnu23
ARFLAGS := -rcs
OUT_A := $(OUTDIR)/libk.a
OBJDIR := $(OBJ_BASE_DIR)/libk
SUBDIRS = $(sort $(dir $(wildcard */*.*)))
OSUBDIRS = $(addprefix $(OBJDIR)/,$(SUBDIRS))
ASUBDIRS = $(addprefix $(ASMT_DIR)/,$(SUBDIRS))
SSRC = $(wildcard *.S */*.S)
CSRC = $(wildcard *.c */*.c)
CPPSRC = $(wildcard *.cpp */*.cpp)
CXXFLAGS += -D__KERNEL__ 
CCFLAGS += -D__KERNEL__ 
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.S=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
ASM_TEST = $(addprefix $(ASMT_DIR)/, $(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/, $(CSRC:%.c=%.test.asm))
SYM_LOG := $(LOG_DIR)/libk-syms.txt
.PHONY: all
all: $(OBJDIR) $(OSUBDIRS) $(OUT_A) $(SYM_LOG)
$(OBJDIR) $(OSUBDIRS) $(ASMT_DIR) $(ASUBDIRS):
	@mkdir -p $@
$(OUT_A): $(OBJECTS)
	@mkdir -p $(PROJECT_DIR)/common/include/asm-generated
	$(PROJECT_DIR)/headergen/build/headergen > $(PROJECT_DIR)/common/include/asm-generated/asm-defs.inc
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.S
	$(AS) -c $< -o $@
$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(ASMT_DIR)/%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
$(ASMT_DIR)/%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
$(SYM_LOG): $(OUT_A)
	$(READ) $(READFLAGS) $< > $@
asmtest: $(ASMT_DIR) $(ASUBDIRS) $(ASM_TEST)