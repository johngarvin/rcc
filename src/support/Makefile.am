lib_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES = \
	parser.cc parser.h\
	util.cc util.h

MYINCLUDES =  -I@R_HOME@/include -I@R_SOURCES@/src/main -I..

libsupport_la_CXXFLAGS  = -static $(BASE_CFLAGS) $(MYINCLUDES)
