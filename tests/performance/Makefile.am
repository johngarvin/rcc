.SECONDARY:  # don't delete intermediate .c files

MYTESTFILES = $(wildcard *.r)

EXTRA_DIST = run-test.in $(MYTESTFILES)

check_SCRIPTS = run-test

MOSTLYCLEANFILES = $(subst .r,.so,$(MYTESTFILES)) $(subst .r,.c,$(MYTESTFILES))

.r.c: ../../src/rcc
	../../src/rcc $<

TEST_CFLAGS  = $(CFLAGS) -I$(R_SOURCES)/src -I$(R_SOURCES)/src/include -I$(top_srcdir)/lib

.c.so: ../../lib/.libs/librcc.so
	$(LIBTOOL) --mode=compile $(CC) -prefer-pic $(CPPFLAGS) $(TEST_CFLAGS) $(LDFLAGS) -shared ../../lib/.libs/librcc.so  $<
	mv -f $(subst .c,.lo,$<) $(subst .c,.so,$<)
