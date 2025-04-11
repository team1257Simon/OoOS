AS := $(TARGET_HOSTED)-as
CXX := $(TARGET_HOSTED)-g++
AR := $(TARGET_HOSTED)-ar
CC := $(TARGET_HOSTED)-gcc
LD := $(TARGET_HOSTED)-ld
READ = $(TARGET_HOSTED)-readelf
OUTDIR = $(IMAGE_FILE_DIR)
# SYS_LIB = $(IMAGE_FILE_DIR)/lib
CFLAGS = -O2 -Wall # -fPIC
CCFLAGS = $(CFLAGS) -std=gnu23
CXXFLAGS = $(CFLAGS) -std=c++20
LDFLAGS = -L$(SYS_LIB) -L$(LIB_GCC) -lc -lgcc -static -lg -lm
TARGET = $(OUTDIR)/test.elf
CSRC = test.c
CPPSRC = $(wildcard *.cpp)
OBJECTS = $(SYS_LIB)/crt0.o $(SYS_LIB)/crti.o $(LIB_GCC)/crtbegin.o $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o) $(LIB_GCC)/crtend.o $(SYS_LIB)/crtn.o 
SYM_LOG := $(LOG_DIR)/test-syms.txt
.PHONY: all
%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(TARGET): $(OBJECTS) 
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@ 
	@mkdir -p $(LOG_DIR)
	$(TARGET_HOSTED)-readelf $(READFLAGS) $@ > $(SYM_LOG)
all: $(OBJECTS) $(OUTDIR) $(TARGET)
clean:
	rm *.o || true
	rm $(TARGET) || true
$(OUTDIR):
	@mkdir -p $@