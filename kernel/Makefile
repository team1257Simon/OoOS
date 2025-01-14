OUTDIR := $(BUILD_DIR)/sys
OBJDIR := $(CURDIR)/build
KINCLUDES := $(INCLUDES)/kernel
CFLAGS := -ffreestanding -O2 -Wall -mno-red-zone $(addprefix -I,$(INCLUDES)) $(addprefix -I,$(KINCLUDES)) -Wno-packed-bitfield-compat
CXXFLAGS := $(CFLAGS) $(addprefix -I,$(INCLUDES_CXX)) -std=c++20
CCFLAGS := $(CFLAGS) -std=gnu17
LDSCRIPT := $(CURDIR)/linker.lds
LDFLAGS := $(CFLAGS) -L$(LIB_DIR) -nostdlib -lk -lgcc
AS := $(ARCH)-$(FMT)-as
CXX := $(ARCH)-$(FMT)-g++
LD := $(ARCH)-$(FMT)-gcc
CC := $(ARCH)-$(FMT)-gcc
FC := $(ARCH)-$(FMT)-ld
COPY := $(ARCH)-$(FMT)-objcopy
OUT_ELF := $(OUTDIR)/core.elf
SUBDIRS = $(sort $(dir $(wildcard */*.* */*/*.*)))
OSUBDIRS = $(addprefix $(OBJDIR)/,$(SUBDIRS))
ASUBDIRS = $(addprefix $(ASMT_DIR)/,$(SUBDIRS))
LSUBDIRS = $(addprefix $(LOG_DIR)/,$(SUBDIRS))
SSRC = $(wildcard *.s */*.s */*/*.s)
CSRC = $(wildcard *.c */*.c */*/*.c)
CPPSRC = $(wildcard *.cpp */*.cpp */*/*.cpp)
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.s=%.o) 
REORDER_OBJ_NAMES_START = crti.o crtbegin.o
REORDER_OBJ_NAMES_END = crtend.o crtn.o
REORDER_OBJ_NAMES_ALL = $(REORDER_OBJ_NAMES_START) $(REORDER_OBJ_NAMES_END)
OBJECTS_REORDERED =  $(filter-out $(REORDER_OBJ_NAMES_ALL), $(OBJECT_NAMES))
OBJECTS = $(addprefix $(LIB_DIR)/,$(REORDER_OBJ_NAMES_START)) $(addprefix $(OBJDIR)/,$(OBJECTS_REORDERED)) $(addprefix $(LIB_DIR)/,$(REORDER_OBJ_NAMES_END))
OBJ_SYMS = $(addprefix $(LOG_DIR)/,$(OBJECT_NAMES:%.o=%-syms.txt))
CRTBEGIN := $(shell $(CC) $(CCFLAGS) -print-file-name=crtbegin.o)
CRTEND := $(shell $(CC) $(CCFLAGS) -print-file-name=crtend.o)
ASMT_DIR := $(CURDIR)/asm_test
ASM_TEST = $(addprefix $(ASMT_DIR)/, $(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/, $(CSRC:%.c=%.test.asm))
SYM_LOG = $(LOG_DIR)/ooos-syms.txt
.PHONY: all asmtest
all: $(OUTDIR) $(OSUBDIRS) $(OBJDIR) $(LSUBDIRS) $(OUT_ELF) $(SYM_LOG) $(OBJ_SYMS)
asmtest: $(ASMT_DIR) $(ASM_TEST)
$(OUTDIR):
	@mkdir -p $@
	@mkdir -p $@/EFI/BOOT
$(OBJDIR) $(LSUBDIRS) $(OSUBDIRS) $(ASMT_DIR) $(ASUBDIRS):
	@mkdir -p $@
$(LIB_DIR)/crtbegin.o:
	$(COPY) -p $(CRTBEGIN) $@
$(LIB_DIR)/crtend.o:
	$(COPY) -p $(CRTEND) $@
$(LIB_DIR)/%.o: %.s
	$(AS) $< -o $@
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.s
	$(AS) $< -o $@
$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(ASMT_DIR)/%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
$(ASMT_DIR)/%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
$(LOG_DIR)/%-syms.txt: $(OBJDIR)/%.o
	$(READ) $(READFLAGS) $< > $@
clean:
	rm $(OUT_ELF) || true
	rm -rf asm_test || true
	rm -rf $(OBJDIR) || true
$(OUT_ELF): $(OBJECTS) $(LDSCRIPT)
	$(LD) -T $(LDSCRIPT) $(OBJECTS) -o $@ $(LDFLAGS)
$(SYM_LOG): $(OUT_ELF)
	$(READ) $(READFLAGS) $< > $@  