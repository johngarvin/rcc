# SUBDIRS = iterators trees

lib_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES = \
	Parser.cc Parser.h \
	StringUtils.cc StringUtils.h \
	\
	iterators/IteratorStack.cc \
	iterators/StackableIterator.cc \
	\
	stacks/StackTmpl.h \
	stacks/PointerStack.C stacks/PointerStack.h \
	\
	trees/NonUniformDegreeTree.cc \
	trees/NonUniformDegreeTree.h \
	trees/NonUniformDegreeTreeTmpl.h 

MYINCLUDES = -I@R_SOURCES@/src/include -I$(top_srcdir)/src

libsupport_la_CXXFLAGS  = -static $(BASE_CFLAGS) $(MYINCLUDES)

libsupport_la_LIBADD = -L@R_LIB_PATH@ -lR
