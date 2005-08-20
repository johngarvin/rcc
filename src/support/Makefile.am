lib_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES =              \
	Parser.cc Parser.h           \
	StringUtils.cc StringUtils.h

MYINCLUDES = -I@R_HOME@/include -I@R_SOURCES@/src/include -I..

libsupport_la_CXXFLAGS  = -static $(BASE_CFLAGS) $(MYINCLUDES)

libsupport_la_LIBADD = -L@R_LIB_PATH@ -lR
