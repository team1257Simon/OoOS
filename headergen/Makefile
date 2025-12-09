CCLD		= g++
SRC			= $(wildcard *.cpp)
OBJDIR 		:= $(CURDIR)/build
CCLDFLAGS	:= -O2 -Wall -std=gnu++26 -freflection
OUTDIR		= $(INCLUDES)/asm-generated
GEN_PRG		= $(OBJDIR)/headergen
OUT_INC		= $(OUTDIR)/$(ARCH)-defs.inc
$(GEN_PRG): $(OBJDIR) $(OUTDIR) $(SRC)
	$(CCLD) $(CCLDFLAGS) $(SRC) -o $@
	$(GEN_PRG) > $(OUT_INC)
$(OBJDIR):
	@mkdir -p $@
$(OUTDIR):
	@mkdir -p $@
all: $(OUTDIR) $(GEN_PRG)
clean:
	rm -rf $(OBJDIR)
.PHONY: all