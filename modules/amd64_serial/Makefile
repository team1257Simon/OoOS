OBJDIR := $(OBJ_BASE_DIR)/amd64_serial
CSRC = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
SSRC = $(wildcard *.S)
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.S=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
FULL_LD_INPUT = $(PRE_OBJS) $(OBJECTS) $(LIB_DIR)/libmodobj.a $(POST_OBJS)
SYM_LOG = $(LOG_DIR)/amd64_serial.syms.txt
OUT_MOD = $(OUTDIR)/amd64_serial.ko
ASM_TEST = $(addprefix $(ASMT_DIR)/, $(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/, $(CSRC:%.c=%.test.asm))
.PHONY: all
all: $(OUTDIR) $(OBJDIR) $(OUT_MOD) $(SYM_LOG)
$(OBJDIR) $(OUTDIR) $(ASMT_DIR) $(ASUBDIRS):
	@mkdir -p $@
$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.S
	$(AS) -c $< -o $@
$(OUT_MOD): $(OBJECTS)
	$(LD) $(FULL_LD_INPUT) -o $@ $(LDFLAGS)
	$(ST) -s $@
$(SYM_LOG): $(OUT_MOD)
	$(READ) $(READFLAGS) $< > $@
$(ASMT_DIR)/%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
$(ASMT_DIR)/%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
asmtest: $(ASMT_DIR) $(ASUBDIRS) $(ASM_TEST)
clean:
	rm $(OUT_MOD) || true