OBJDIR := $(OBJ_BASE_DIR)/test_module
CSRC = $(wildcard *.c)
CPPSRC = $(wildcard *.cpp)
SSRC = $(wildcard *.S)
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.S=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
FULL_LD_INPUT = $(PRE_OBJS) $(OBJECTS) $(LIB_DIR)/libmodobj.o $(POST_OBJS)
SYM_LOG = $(LOG_DIR)/test_module.syms.txt
OUT_MOD = $(OUTDIR)/test_module.ko
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
$(SYM_LOG): $(OUT_MOD)
	$(READ) $(READFLAGS) $< > $@