
.r.c:
	../src/rcc $<

rcc_CFLAGS  = $(BASE_CFLAGS) -I$(R_SOURCES)/src/include -I$(top_srcdir)/lib

.c.so: ../lib/.libs/librcc.so
	$(CC) $(CPPFLAGS) $(rcc_CFLAGS) $(HI_OPTFLAGS) $(LDFLAGS) -fpic -shared ../lib/.libs/librcc.so  $< -o $@

check: ../src/rcc ../lib/.libs/librcc.so
	./run-test
