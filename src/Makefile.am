
bin_PROGRAMS = rcc

rcc_SOURCES = \
	      rcc.cc rcc.h \
	      get_name.c get_name.h

rcc_CFLAGS  = $(BASE_CFLAGS) -I$(R_SOURCES)/src/include -I$(top_srcdir)/lib
rcc_CXXFLAGS  = $(BASE_CFLAGS) -I$(R_SOURCES)/src/include -I$(top_srcdir)/lib
rcc_LDFLAGS =-L$(R_PREFIX)/lib/R/bin -lR -lstdc++

