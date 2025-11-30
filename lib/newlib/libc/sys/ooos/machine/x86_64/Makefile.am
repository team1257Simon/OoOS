AUTOMAKE_OPTIONS = cygnus

INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)
if USE_LIBTOOL
AM_CCASFLAGS = $(INCLUDES)
else
AM_CCASFLAGS = $(INCLUDES)
endif

LIB_SOURCES = tls_stub.c dl_stub.c

libooosx86_64_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libooosx86_64.la
libooosx86_64_la_SOURCES = $(LIB_SOURCES)
noinst_DATA = objectlist.awk.in
else
noinst_LIBRARIES = lib.a

lib_a_SOURCES = $(LIB_SOURCES)
lib_a_LIBADD =
lib_a_DEPENDENCIES =
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)
noinst_DATA =
endif

all-local: crt0.o crti.o crtn.o

crt0.o: crt0.S
	$(LTCOMPILE) -DDYNAMIC -c $< -o $@

crti.o: crti.S
	$(LTCOMPILE) -c $< -o $@
    
crtn.o: crtn.S
	$(LTCOMPILE) -c $< -o $@

include $(srcdir)/../../../../../Makefile.shared

ACLOCAL_AMFLAGS = -I ../../../../.. -I ../../../../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host