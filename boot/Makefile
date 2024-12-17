TARGET := $(BUILD_DIR)/EFI/BOOT/BOOTX64.EFI
CSRC := $(wildcard %.c)
OBJS := $(CSRC: %.c=%.o)
CFLAGS = -DUEFI_NO_TRACK_ALLOC
include uefi/Makefile