AS := $(TARGET_HOSTED)-as
CXX := $(TARGET_HOSTED)-g++
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-ld
READ = $(TARGET_HOSTED)-readelf
CFLAGS = -O2 -Wall
CCFLAGS = $(CFLAGS) -std=gnu23
CXXFLAGS = $(CFLAGS)-std=c++20
LDFLAGS = $(CFLAGS) -shared -L$(SYS_LIB) -L$(LIB_GCC) -lc -lg -lm -lnosys
OUTDIR = $(IMAGE_FILE_DIR)/lib
OBJDIR := $(OBJ_BASE_DIR)/ld-ooos
SSRC = $(wildcard *.S */*.S)
CSRC = $(wildcard *.c */*.c)
CPPSRC = $(wildcard *.cpp */*.cpp)
OBJECT_NAMES = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(SSRC:%.S=%.o)
OBJECTS = $(addprefix $(OBJDIR)/,$(OBJECT_NAMES))
ASM_TEST = $(addprefix $(ASMT_DIR)/, $(CPPSRC:%.cpp=%.test.asm)) $(addprefix $(ASMT_DIR)/, $(CSRC:%.c=%.test.asm))
OUT_SO = $(OUTDIR)/ld-ooos.so
SYM_LOG := $(LOG_DIR)/ld-ooos-syms.txt
.PHONY: all
all: $(OBJDIR) $(OUTDIR) $(OUT_SO) $(SYM_LOG)
$(OUTDIR) $(OBJDIR) $(ASMT_DIR):
	@mkdir -p $@
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(OBJDIR)/%.o: %.S
	$(AS) $< -o $@
$(OBJDIR)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(ASMT_DIR)/%.test.asm: %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@
$(ASMT_DIR)/%.test.asm: %.c
	$(CC) $(CCFLAGS) -S $< -o $@
$(SYM_LOG): $(OUT_SO)
	$(READ) $(READFLAGS) $< > $@
$(OUT_SO): $(OBJECTS)
	$(LD) $(SYS_LIB)/crti.o $(LIB_GCC)/crtbegin.o $(OBJECTS) $(LDFLAGS) $(LIB_GCC)/crtend.o $(SYS_LIB)/crtn.o -o $@
	@mkdir -p $(LOG_DIR)
	$(TARGET_HOSTED)-readelf $(READFLAGS) $@ > $(SYM_LOG)
asmtest: $(ASMT_DIR) $(ASM_TEST)
