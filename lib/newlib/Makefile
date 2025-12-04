
CC 				:= $(TARGET_HOSTED)-gcc
READ 			:= $(TARGET_HOSTED)-readelf
LIB_OUT_DIR		:= $(IMAGE_FILE_DIR)/lib
LIBC_SYMLINK	= $(shell $(CC) --print-file-name=libc.so)
LIBM_SYMLINK	= $(shell $(CC) --print-file-name=libm.so)
LIBC_DIR		= $(dir $(LIBC_SYMLINK))
LIBM_DIR		= $(dir $(LIBM_SYMLINK))
LIBC_FILENAME	= $(shell readlink $(LIBC_SYMLINK))
LIBM_FILENAME	= $(shell readlink $(LIBM_SYMLINK))
LIBC_FILE		= $(addprefix $(LIBC_DIR),$(LIBC_FILENAME))
LIBM_FILE		= $(addprefix $(LIBM_DIR),$(LIBM_FILENAME))
LIBC_MINORV		= $(basename $(LIBC_FILENAME))
LIBM_MINORV		= $(basename $(LIBM_FILENAME))
LIBC_MAJORV		= $(basename $(LIBC_MINORV))
LIBM_MAJORV		= $(basename $(LIBM_MINORV))
LIBC_DIST		= $(basename $(LIBC_MAJORV))
LIBM_DIST		= $(basename $(LIBM_MAJORV))
IMG_LIBC		= $(addprefix $(LIB_OUT_DIR)/,$(LIBC_FILENAME))
IMG_LIBM		= $(addprefix $(LIB_OUT_DIR)/,$(LIBM_FILENAME))
LINKS_LIBC		= $(LIBC_MINORV) $(LIBC_MAJORV) $(LIBC_DIST) libc.so
LINKS_LIBM		= $(LIBM_MINORV) $(LIBM_MAJORV) $(LIBM_DIST) libm.so
LINKS			= $(LINKS_LIBC) $(LINKS_LIBM)
IMG_LINKS		= $(addprefix $(LIB_OUT_DIR)/,$(LINKS))
IMG_LINKS_LIBC	= $(addprefix $(LIB_OUT_DIR)/,$(LINKS_LIBC))
IMG_LINKS_LIBM	= $(addprefix $(LIB_OUT_DIR)/,$(LINKS_LIBM))
SYM_LOG_LIBC 	:= $(LOG_DIR)/libc-syms.txt
SYM_LOG_LIBM 	:= $(LOG_DIR)/libm_syms.txt
.PHONY: all clean
all: $(LIB_OUT_DIR) $(IMG_LIBC) $(IMG_LIBM) $(IMG_LINKS) $(SYM_LOG_LIBC) $(SYM_LOG_LIBM)
$(LIB_OUT_DIR) $(LOG_DIR):
	@mkdir -p $@
$(IMG_LIBC): $(LIBC_FILE)
	@cp $< $@
$(IMG_LIBM): $(LIBM_FILE)
	@cp $< $@
$(IMG_LINKS_LIBC): $(IMG_LIBC)
	ln -sr $(IMG_LIBC) $@
$(IMG_LINKS_LIBM): $(IMG_LIBM)
	ln -sr $(IMG_LIBM) $@
$(SYM_LOG_LIBC): $(IMG_LIBC) $(LOG_DIR)
	$(READ) $(READFLAGS) $< > $@
$(SYM_LOG_LIBM): $(IMG_LIBM) $(LOG_DIR)
	$(READ) $(READFLAGS) $< > $@
clean:
	true