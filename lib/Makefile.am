
lib_LTLIBRARIES = librcc.la

include_HEADERS = \
	rcc_lib.h replacements.h

nodist_include_HEADERS = \
	$(R_SOURCES)/src/include/IOStuff.h	\
	$(R_SOURCES)/src/include/Parse.h	\
	$(R_SOURCES)/src/include/Internal.h	\
	$(R_SOURCES)/src/main/arithmetic.h

librcc_la_SOURCES = \
	rcc_lib.c \
	replacements.c

librcc_la_CFLAGS  = $(BASE_CFLAGS) -I$(R_SOURCES)/src/include -I$(R_SOURCES)/src/main -I.

