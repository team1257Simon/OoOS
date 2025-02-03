AS := $(TARGET_HOSTED)-as
CXX := $(TARGET_HOSTED)-g++
AR := $(TARGET_HOSTED)-ar
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-ld
CFLAGS = -O2 -Wall
CCFLAGS = $(CFLAGS) -std=gnu23
CXXFLAGS = $(CFLAGS) -std=c++20
LDFLAGS = -lc -lg -lm -lnosys
OUTDIR = $(BUILD_DIR)/FILES
TARGET = $(OUTDIR)/TEST.ELF
CSRC = test.c
CPPSRC = $(wildcard *.cpp)
OBJECTS = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o)
.PHONY: all
%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(TARGET): $(OBJECTS) 
	$(LD) /usr/local/ooos/sysroot/usr/lib/crt0.o $(OBJECTS) -o $@ $(LDFLAGS)
all: $(OBJECTS) $(OUTDIR) $(TARGET)
clean:
	rm *.o || true
	rm $(TARGET) || true
$(OUTDIR):
	@mkdir -p $@