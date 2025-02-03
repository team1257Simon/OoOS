AS := $(TARGET_BARE)-as
CXX := $(TARGET_BARE)-g++
AR := $(TARGET_BARE)-ar
CC := $(TARGET_BARE)-gcc
OUT_A := $(OUTDIR)/libk.a
OBJDIR := $(OBJ_BASE_DIR)/libk
SUBDIRS = $(sort $(dir $(wildcard */*.*)))
OSUBDIRS = $(addprefix $(OBJDIR)/,$(SUBDIRS))
ASUBDIRS = $(addprefix $(ASMT_DIR)/,$(SUBDIRS))
SSRC = $(wildcard *.s */*.s)
CSRC = $(wildcard *.c */*.c)
CPPSRC = $(wildcard *.cpp */*.cpp)
CXXFLAGS += -D__KERNEL__ 
CCFLAGS += -D__KERNEL__ 
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.s=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
ASM_TEST = $(addprefix $(ASMT_DIR)/, $(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/, $(CSRC:%.c=%.test.asm))
SYM_LOG := $(LOG_DIR)/libk-syms.txt
.PHONY: all
all: $(OBJDIR) $(OSUBDIRS) $(OUT_A) $(SYM_LOG)
$(OBJDIR) $(OSUBDIRS) $(ASMT_DIR) $(ASUBDIRS):
	@mkdir -p $@
$(OUT_A): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)
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
$(SYM_LOG): $(OUT_A)
	$(READ) $(READFLAGS) $< > $@
asmtest: $(ASMT_DIR) $(ASUBDIRS) $(ASM_TEST)