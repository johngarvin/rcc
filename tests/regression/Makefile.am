.SECONDARY:  # don't delete intermediate .c files

MYTESTFILES = $(wildcard *.r)

EXTRA_DIST = run-test.in $(MYTESTFILES)

src_SCRIPTS = run-test.in

MOSTLYCLEANFILES = $(subst .r,.so,$(MYTESTFILES)) $(subst .r,.c,$(MYTESTFILES))

TEST_CFLAGS  = $(CFLAGS) -I$(R_SOURCES)/src -I$(R_SOURCES)/src/include -I$(top_srcdir)/lib

%.c: %.r ../../src/rcc
	../../src/rcc $<

%.so: %.c ../../src/rcc
	$(LIBTOOL) --mode=compile $(CC) -prefer-pic $(CPPFLAGS) $(TEST_CFLAGS) $(LDFLAGS) -shared ../../lib/.libs/librcc.so $<
	mv -f $(<:.c=.o) $(<:.c=.so)

check: run-test ../../src/rcc ../../lib/.libs/librcc.so
	sh ./run-test
