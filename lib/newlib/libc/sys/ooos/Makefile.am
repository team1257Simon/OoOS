AUTOMAKE_OPTIONS = cygnus
INCLUDES = $(NEWLIB_CFLAGS) $(CROSS_CFLAGS) $(TARGET_CFLAGS)
AM_CCASFLAGS = $(INCLUDES)

SUBDIRS = machine \
	.

noinst_LIBRARIES = lib.a

SUBLIBS = $(OOOS_MACHINE_LIB) \
	$(EXTRA_SUBLIBS)

GENERAL_SOURCES = syscalls.c posix_stub.c dirent.c
ADD_OBJS = $(OOOS_MACHINE_LIB)

# libooos_la_LDFLAGS = -Xcompiler -nostdlib

if USE_LIBTOOL
noinst_LTLIBRARIES = libooos.la
libooos_la_SOURCES = $(GENERAL_SOURCES)
libooos_la_LIBADD = $(ADD_OBJS)
libooos_la_DEPENDENCIES = $(ADD_OBJS)
LIBC_COMPILE = $(LTCOMPILE)
noinst_DATA = objectlist.awk.in
else
lib_a_SOURCES = $(GENERAL_SOURCES)
lib_a_LIBADD = $(ADD_OBJS)
lib_a_DEPENDENCIES = $(ADD_OBJS)
lib_a_CCASFLAGS = $(AM_CCASFLAGS)
lib_a_CFLAGS = $(AM_CFLAGS)
LIBC_COMPILE = $(COMPILE)
noinst_DATA =

lib.a: $(lib_a_OBJECTS)
	rm -f $@
	rm -rf tmp
	mkdir tmp
	cd tmp; \
	 for i in $(SUBLIBS); do \
	   $(AR) x ../$$i; \
	 done; \
	$(AR) $(AR_FLAGS) ../$@ *.o ../lib_a-*.o
	$(RANLIB) $@
	rm -rf tmp

endif # USE_LIBTOOL

objectlist.awk.in: $(noinst_LTLIBRARIES) $(SUBLIBS)
	-rm -f objectlist.awk.in
	for i in `ls *.lo` ; \
	  do \
		echo $$i `pwd`/$$i >> objectlist.awk.in ; \
	done
	for j in $(SUBLIBS) ; \
	  do \
		export i=`echo $$j | sed -e 's,\(.*\)\/[^\/]*$$,\1,'`; \
		if test $$i != "." && test -f $$i/objectlist.awk.in; then \
			cat $$i/objectlist.awk.in >> objectlist.awk.in ; \
		fi; \
	done

all-local: crt0.o crti.o crtn.o

crt0.o: machine/crt0.o
	rm -f $@
	ln machine/crt0.o $@ >/dev/null 2>/dev/null || cp machine/crt0.o $@
crti.o: machine/crti.o
	rm -f $@
	ln machine/crti.o $@ >/dev/null 2>/dev/null || cp machine/crti.o $@
crtn.o: machine/crtn.o
	rm -f $@
	ln machine/crtn.o $@ >/dev/null 2>/dev/null || cp machine/crtn.o $@

machine/crt0.o:; @true
machine/crti.o:; @true
machine/crtn.o:; @true
machine/lib.a:; @true

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/arpa; \
	for i in $(srcdir)/include/arpa/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/arpa/`basename $$i`; \
	done; \
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/netinet; \
	for i in $(srcdir)/include/netinet/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/netinet/`basename $$i`; \
	done;
	$(mkinstalldirs) $(DESTDIR)$(tooldir)/include/net; \
	for i in $(srcdir)/include/net/*.h; do \
	  $(INSTALL_DATA) $$i $(DESTDIR)$(tooldir)/include/net/`basename $$i`; \
	done;

ACLOCAL_AMFLAGS = -I ../../..
CONFIG_STATUS_DEPENDENCIES = $(newlib_basedir)/configure.host