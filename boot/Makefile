TARGET := $(BUILD_DIR)/EFI/BOOT/BOOTX64.EFI
CSRC := $(wildcard %.c)
OBJS := $(CSRC: %.c=%.o)
CFLAGS = -DUEFI_NO_TRACK_ALLOC -std=gnu2x -DNEED_STDBOOL
include uefi/Makefile