bin_PROGRAMS = rcc

rcc_SOURCES = \
	      rcc.cc rcc.h \
              util.cc util.h \
              R_parser.cc R_parser.h \
	      get_name.c get_name.h

rcc_LIBRARIES = ../lib/librcc.so

rcc_CFLAGS  = $(BASE_CFLAGS) -I$(R_SOURCES)/src -I$(R_SOURCES)/src/include -I$(top_srcdir)/lib -DR_HOME=\"$(R_PREFIX)\"
rcc_CXXFLAGS= $(BASE_CFLAGS) -I$(R_SOURCES)/src -I$(R_SOURCES)/src/include -I$(top_srcdir)/lib -DR_HOME=\"$(R_PREFIX)\"
rcc_LDFLAGS =-L$(R_PREFIX)/lib/R/bin -lR

