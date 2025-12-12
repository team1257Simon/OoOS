OBJDIR := $(OBJ_BASE_DIR)/e1000e
CSRC = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
SSRC = $(wildcard *.S)
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.S=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
FULL_LD_INPUT = $(PRE_OBJS) $(OBJECTS) $(LIB_DIR)/libmodobj.a $(POST_OBJS)
SYM_LOG = $(LOG_DIR)/e1000e.syms.txt
OUT_MOD = $(OUTDIR)/IE1000E.KO
ASM_TEST = $(CPPSRC:%.cpp=%.test.asm) $(CSRC:%.c=%.test.asm)
.PHONY: all
all: $(OUTDIR) $(OBJDIR) $(OUT_MOD) $(SYM_LOG)
$(OBJDIR) $(OUTDIR):
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
%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
asmtest: $(ASM_TEST)
clean:
	rm $(OUT_MOD) || true
	rm $(ASM_TEST) || true