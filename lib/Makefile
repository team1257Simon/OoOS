OUTDIR := $(LIB_DIR)
OBJ_BASE_DIR := $(CURDIR)/build
export OUTDIR
export OBJ_BASE_DIR
export TARGET_BARE
export TARGET_HOSTED
SUBDIRS = libk libmodobj ld-ooos newlib
.PHONY: all $(SUBDIRS)
all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@
clean:
	rm -rf $(OBJ_BASE_DIR) || true
	rm -rf $(OUTDIR)/*.a $(OUTDIR)/*.o || true
	rm -rf $(IMAGE_FILE_DIR)/lib/*.* || true
	for dir in $(SUBDIRS); do \
		cd $$dir; \
		$(MAKE) clean ; \
		cd .. ; \
	done
asmtest:
	cd libk && $(MAKE) asmtest
	cd ld-ooos && $(MAKE) asmtest
	cd libmodobj && $(MAKE) asmtest