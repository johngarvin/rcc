.SECONDARY:  # don't delete intermediate .c files

MYTESTFILES = $(wildcard *.r)

EXTRA_DIST = run-test.in $(MYTESTFILES)

src_SCRIPTS = run-test.in

MOSTLYCLEANFILES = $(subst .r,.so,$(MYTESTFILES)) $(subst .r,.c,$(MYTESTFILES))

.r.c: ../../src/rcc
	../../src/rcc $<

TEST_CFLAGS  = $(CFLAGS) -I$(R_SOURCES)/src -I$(R_SOURCES)/src/include -I$(top_srcdir)/lib

.c.so: ../../lib/.libs/librcc.so
	$(CC) $(CPPFLAGS) $(TEST_CFLAGS) $(LDFLAGS) -fpic -shared ../../lib/.libs/librcc.so  $< -o $@

check: run-test ../../src/rcc ../../lib/.libs/librcc.so
	sh ./run-test
