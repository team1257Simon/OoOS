OUTDIR := $(BUILD_DIR)/SYS
OBJDIR := $(CURDIR)/build
KINCLUDES := $(INCLUDES)/kernel
CFLAGS := -ffreestanding -O2 -Wall -mno-red-zone $(addprefix -I,$(INCLUDES)) $(addprefix -I,$(KINCLUDES)) -Wno-packed-bitfield-compat -D__KERNEL__ -mcrc32
CXXFLAGS := $(CFLAGS) $(addprefix -I,$(INCLUDES_CXX)) -std=gnu++26 -freflection -nostdinc++ -fplugin=$(ATTR_PLUGIN) -fopenmp
CCFLAGS := $(CFLAGS) -std=gnu23 -fplugin=$(ATTR_PLUGIN)
ASFLAGS := -I$(PROJECT_DIR)/common/include/asm -I$(PROJECT_DIR)/common/include/asm-generated
LDSCRIPT := $(CURDIR)/linker.lds
LDFLAGS := $(CFLAGS) -L$(LIB_DIR) -nostdlib -lk -lgcc
AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
LD := $(TARGET_HOSTED)-gcc
CC := $(TARGET_HOSTED)-gcc
COPY := $(TARGET_HOSTED)-objcopy
OUT_ELF := $(OUTDIR)/CORE.ELF
SUBDIRS = $(sort $(dir $(wildcard */*.* */*/*.*)))
OSUBDIRS = $(addprefix $(OBJDIR)/,$(SUBDIRS))
LSUBDIRS = $(addprefix $(LOG_DIR)/,$(SUBDIRS))
SSRC = $(wildcard *.S */*.S */*/*.S)
CSRC = $(wildcard *.c */*.c */*/*.c)
CPPSRC = $(wildcard *.cpp */*.cpp */*/*.cpp)
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.S=%.o)
REORDER_OBJ_NAMES_START = crti.o crtbegin.o
REORDER_OBJ_NAMES_END = crtend.o crtn.o
REORDER_OBJ_NAMES_ALL = $(REORDER_OBJ_NAMES_START) $(REORDER_OBJ_NAMES_END)
OBJECTS_REORDERED = $(filter-out $(REORDER_OBJ_NAMES_ALL), $(OBJECT_NAMES))
OBJECTS = $(addprefix $(LIB_DIR)/,$(REORDER_OBJ_NAMES_START)) $(addprefix $(OBJDIR)/,$(OBJECTS_REORDERED)) $(addprefix $(LIB_DIR)/,$(REORDER_OBJ_NAMES_END))
OBJ_SYMS = $(addprefix $(LOG_DIR)/,$(OBJECT_NAMES:%.o=%-syms.txt))
CRTBEGIN := $(shell $(CC) $(CCFLAGS) -print-file-name=crtbegin.o)
CRTEND := $(shell $(CC) $(CCFLAGS) -print-file-name=crtend.o)
ASM_TEST = $(CPPSRC:%.cpp=%.test.asm) $(CSRC:%.c=%.test.asm)
SYM_LOG := $(LOG_DIR)/ooos-syms.txt
.PHONY: all asmtest
all: $(OUTDIR) $(OSUBDIRS) $(OBJDIR) $(LSUBDIRS) $(OUT_ELF) $(SYM_LOG) $(OBJ_SYMS)
asmtest: $(ASM_TEST)
$(OUTDIR):
	@mkdir -p $@
	@mkdir -p $@/EFI/BOOT
$(OBJDIR) $(LSUBDIRS) $(OSUBDIRS):
	@mkdir -p $@
$(LIB_DIR)/crtbegin.o:
	$(COPY) -p $(CRTBEGIN) $@
$(LIB_DIR)/crtend.o:
	$(COPY) -p $(CRTEND) $@
$(LIB_DIR)/%.o: %.S
	$(AS) -c $< -o $@
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.S
	$(AS) $(ASFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
$(LOG_DIR)/%-syms.txt: $(OBJDIR)/%.o
	$(READ) $(READFLAGS) $< > $@
clean:
	@rm $(OUT_ELF) || true
	@rm -rf $(OBJDIR) || true
	@rm $(ASM_TEST) > /dev/null 2>/dev/null || true
$(OUT_ELF): $(OBJECTS) $(LDSCRIPT)
	$(LD) -T $(LDSCRIPT) $(OBJECTS) -o $@ $(LDFLAGS)
$(SYM_LOG): $(OUT_ELF)
	$(READ) $(READFLAGS) $< > $@