BUILD 		= x86_64-linux-gnu
CCLD		= $(BUILD)-g++
SRC			= $(wildcard *.cpp)
OBJDIR 		:= $(CURDIR)/build
CFLAGS		= -O2 -Wall
CCLDFLAGS 	:= $(CFLAGS) -std=c++20
OUTDIR		= $(INCLUDES)/asm-generated
GEN_PRG		= $(OBJDIR)/headergen
OUT_INC		= $(OUTDIR)/$(ARCH)-defs.inc
$(GEN_PRG): $(OBJDIR) $(SRC) 
	$(CCLD) $(CCLDFLAGS) $(SRC) -o $@
$(OBJDIR):
	@mkdir -p $@
$(OUTDIR):
	@mkdir -p $@
all: $(OUTDIR) $(GEN_PRG)
	$(GEN_PRG) > $(OUT_INC)
clean:
	rm -rf $(OBJDIR)
.PHONY: all