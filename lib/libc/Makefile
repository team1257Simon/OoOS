OUT_A := $(OUTDIR)/libc.a
OBJDIR := $(OBJ_BASE_DIR)/libc
SSRC = $(wildcard *.s)
CSRC = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.s=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
ASM_TEST = $(addprefix $(ASMT_DIR)/, $(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/, $(CSRC:%.c=%.test.asm))
SYM_LOG := $(LOG_DIR)/libc-syms.txt
.PHONY: all
all: $(OBJDIR) $(OUT_A) $(SYM_LOG)
$(OBJDIR):
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
$(ASMT_DIR):
	@mkdir -p $@
$(SYM_LOG): $(OUT_A)
	$(READ) $(READFLAGS) $< > $@  
asmtest: $(ASMT_DIR) $(ASM_TEST)