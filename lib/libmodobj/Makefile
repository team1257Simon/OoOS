AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-gcc
ST := $(TARGET_HOSTED)-strip
AR := $(TARGET_HOSTED)-ar
READ = $(TARGET_HOSTED)-readelf
COPY := $(TARGET_HOSTED)-objcopy
ARFLAGS := -rcs
LDFLAGS := -r -nostdlib
OBJDIR := $(OBJ_BASE_DIR)/libmodobj
OUT_A := $(OUTDIR)/libmodobj.a
CFLAGS = -O2 -fPIC $(addprefix -I,$(INCLUDES)) -mno-red-zone -Wno-packed-bitfield-compat
CXXFLAGS := $(CFLAGS) -std=gnu++23 $(addprefix -I,$(INCLUDES_CXX)) -nostdinc++ -fvisibility-inlines-hidden
CCFLAGS := $(CFLAGS) -std=gnu23
SUBDIRS = $(sort $(dir $(wildcard */*.*)))
OSUBDIRS = $(addprefix $(OBJDIR)/,$(SUBDIRS))
ASUBDIRS = $(addprefix $(ASMT_DIR)/,$(SUBDIRS))
SSRC = $(wildcard lib*.S)
CRTSRC = $(wildcard mod*.S)
CSRC = $(wildcard *.c */*.c)
CPPSRC = $(wildcard *.cpp */*.cpp)
CRTOBJ_NAMES = $(CRTSRC:%.S=%.o) modcrtbegin.o modcrtend.o
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.S=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
CRTOBJS = $(addprefix $(OBJDIR)/,$(CRTOBJ_NAMES))
CRTOBJ_OUT = $(addprefix $(OUTDIR)/,$(CRTOBJ_NAMES))
ASM_TEST = $(addprefix $(ASMT_DIR)/,$(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/,$(CSRC:%.c=%.test.asm))
SYM_LOG := $(LOG_DIR)/libmodobj-syms.txt
CRTBEGIN := $(shell $(CC) $(CCFLAGS) -print-file-name=crtbegin.o)
CRTEND := $(shell $(CC) $(CCFLAGS) -print-file-name=crtend.o)
.PHONY: all
all: $(OBJDIR) $(OSUBDIRS) $(OUT_A) $(SYM_LOG) $(CRTOBJ_OUT)
$(OBJDIR) $(OSUBDIRS) $(ASMT_DIR) $(ASUBDIRS):
	@mkdir -p $@
$(OUT_A): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(OBJDIR)/libmodobj_rel.o -lstdc++ -lgcc
	$(AR) $(ARFLAGS) $@ $(OBJDIR)/libmodobj_rel.o
	$(ST) -x $@
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.S
	$(AS) -c $< -o $@
$(OUTDIR)/%.o: $(OBJDIR)/%.o
	@cp $< $@
$(OUTDIR)/modcrtbegin.o:
	$(COPY) -p $(CRTBEGIN) $@
$(OUTDIR)/modcrtend.o:
	$(COPY) -p $(CRTEND) $@
$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(ASMT_DIR)/%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
$(ASMT_DIR)/%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
$(SYM_LOG): $(OUT_A)
	$(READ) $(READFLAGS) $< > $@
asmtest: $(ASMT_DIR) $(ASUBDIRS) $(ASM_TEST)