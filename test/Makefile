AS := $(TARGET_HOSTED)-as
CXX := $(TARGET_HOSTED)-g++
AR := $(TARGET_HOSTED)-ar
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-gcc
READ = $(TARGET_HOSTED)-readelf
OUTDIR = $(IMAGE_FILE_DIR)
SYS_LIB = $(IMAGE_FILE_DIR)/lib
CFLAGS = -O2 -Wall -fPIC
CCFLAGS = $(CFLAGS) -std=gnu23
CXXFLAGS = $(CFLAGS) -std=c++20
LDFLAGS = -L$(SYS_LIB) -L$(LIB_GCC) -Wl,--hash-style=both
TARGET = $(OUTDIR)/test.elf
CSRC = test.c
CPPSRC = $(wildcard *.cpp)
OBJECTS = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o)
SYM_LOG := $(LOG_DIR)/test-syms.txt
.PHONY: all
all: $(OBJECTS) $(OUTDIR) $(TARGET) $(SYM_LOG)
%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@
clean:
	rm *.o || true
	rm $(TARGET) || true
$(OUTDIR) $(LOG_DIR):
	@mkdir -p $@
$(SYM_LOG):	$(TARGET)
	$(READ) $(READFLAGS) $< > $@