AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-gcc
ST := $(TARGET_HOSTED)-strip
AR := $(TARGET_HOSTED)-ar
READ = $(TARGET_HOSTED)-readelf
COPY := $(TARGET_HOSTED)-objcopy
ARFLAGS := -rcs
OBJDIR := $(OBJ_BASE_DIR)/libmodobj
OUT := $(OUTDIR)/libmodobj.a
INCLUDES += $(PROJECT_DIR)/common/include/kernel
CFLAGS = -O2 -fPIC -fPIE -mno-red-zone -Wno-packed-bitfield-compat -D__LIBK__ -fopenmp -msse -msse2 -mssse3 -msse4a -msse4
CXXFLAGS := $(CFLAGS) -std=gnu++26 $(addprefix -I,$(INCLUDES)) $(addprefix -I,$(INCLUDES_CXX)) -freflection -nostdinc++ -fvisibility-inlines-hidden -fplugin=$(ATTR_PLUGIN)
CCFLAGS := $(CFLAGS) $(addprefix -I,$(INCLUDES)) -std=gnu23 -fplugin=$(ATTR_PLUGIN)
SUBDIRS = $(sort $(dir $(wildcard */*.*)))
OSUBDIRS = $(addprefix $(OBJDIR)/,$(SUBDIRS))
SSRC = $(wildcard lib*.S)
CRTSRC = $(wildcard mod*.S)
CSRC = $(wildcard *.c */*.c)
CPPSRC = $(wildcard *.cpp */*.cpp)
CRTOBJ_NAMES = $(CRTSRC:%.S=%.o) modcrtbegin.o modcrtend.o
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.S=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
CRTOBJS = $(addprefix $(OBJDIR)/,$(CRTOBJ_NAMES))
CRTOBJ_OUT = $(addprefix $(OUTDIR)/,$(CRTOBJ_NAMES))
ASM_TEST = $(CPPSRC:%.cpp=%.test.asm) $(CSRC:%.c=%.test.asm)
SYM_LOG := $(LOG_DIR)/libmodobj-syms.txt
CRTBEGIN := $(shell $(CC) $(CCFLAGS) -print-file-name=crtbeginS.o)
CRTEND := $(shell $(CC) $(CCFLAGS) -print-file-name=crtendS.o)
.PHONY: all clean
all: $(OBJDIR) $(OSUBDIRS) $(OUT) $(SYM_LOG) $(CRTOBJ_OUT)
$(OBJDIR) $(OSUBDIRS):
	@mkdir -p $@
$(OUT): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
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
%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
$(SYM_LOG): $(OUT)
	$(READ) $(READFLAGS) $< > $@
asmtest: $(ASM_TEST)
clean:
	rm $(ASM_TEST) > /dev/null 2>/dev/null || true