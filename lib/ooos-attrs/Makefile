TARGET		:=	x86_64-ooos
HOST_GCC	:=	g++
TARGET_GCC	:=	$(TARGET)-gcc
PLUGINS_DIR	:= 	$(shell $(TARGET_GCC) -print-file-name=plugin)
PINCLUDES	:=	$(PLUGINS_DIR)/include
CXXFLAGS	:=	-O2 -fno-rtti -fPIC -std=gnu++26 $(addprefix -I,$(PINCLUDES)) -Wno-deprecated-enum-enum-conversion
SOURCES		:= $(wildcard *.cpp)
OUT_SO		:= $(ATTR_PLUGIN)

$(OUT_SO): $(SOURCES)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@

.PHONY: all clean
all: $(OUT_SO)
clean:
	rm -rf *.o *.so || true