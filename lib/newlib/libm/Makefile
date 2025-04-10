AS := $(TARGET_HOSTED)-gcc
CXX := $(TARGET_HOSTED)-g++
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-gcc
ST := $(TARGET_HOSTED)-strip
READ = $(TARGET_HOSTED)-readelf
CFLAGS = -O2 -fPIC -ffreestanding
CCFLAGS = $(CFLAGS) -std=gnu23 -w
CXXFLAGS = $(CFLAGS) -std=c++20 -w
ARFLAGS := -rcs
OUTDIR = $(IMAGE_FILE_DIR)/lib
OBJDIR := $(OBJ_BASE_DIR)/libc/libm
SUBDIRS = $(sort $(dir $(wildcard */*.*)))
OSUBDIRS = $(addprefix $(OBJDIR)/,$(SUBDIRS))
ASUBDIRS = $(addprefix $(ASMT_DIR)/,$(SUBDIRS))
CSRC = $(wildcard *.c */*.c */*/*.c)
CPPSRC = $(wildcard *.cpp */*.cpp)
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES)) 
ASM_TEST = $(addprefix $(ASMT_DIR)/, $(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/, $(CSRC:%.c=%.test.asm))
OUT_A = $(OUTDIR)/libm.so
SYM_LOG := $(LOG_DIR)/libm_syms.txt
.PHONY: all
all: $(OBJDIR) $(OSUBDIRS) $(OUTDIR) $(OUT_A) $(SYM_LOG)
$(OUTDIR) $(OBJDIR) $(OSUBDIRS) $(ASMT_DIR) $(LOG_DIR):
	@mkdir -p $@
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.S
	$(AS) -fPIC -c $< -o $@
$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(ASMT_DIR)/%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
$(ASMT_DIR)/%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
$(SYM_LOG): $(OUT_A) $(LOG_DIR) 
	$(READ) $(READFLAGS) $< > $@
$(OUT_A): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
asmtest: $(ASMT_DIR) $(ASM_TEST)
