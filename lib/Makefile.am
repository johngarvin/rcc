
lib_LTLIBRARIES = librcc.la

include_HEADERS = \
	rcc_lib.h replacements.h

nodist_include_HEADERS = \
	$(R_SOURCES)/src/include/Defn.h		\
	$(R_SOURCES)/src/include/Errormsg.h	\
	$(R_SOURCES)/src/include/Internal.h	\
	$(R_SOURCES)/src/include/IOStuff.h	\
	$(R_SOURCES)/src/include/Parse.h	\
	$(R_SOURCES)/src/include/Rinternals.h

include_R_extdir = ${includedir}/R_ext
nodist_include_R_ext_HEADERS = \
	$(R_SOURCES)/src/include/R_ext/Arith.h	\
	$(R_SOURCES)/src/include/R_ext/Boolean.h \
	$(R_SOURCES)/src/include/R_ext/Complex.h \
	$(R_SOURCES)/src/include/R_ext/Error.h	\
	$(R_SOURCES)/src/include/R_ext/Memory.h	\
	$(R_SOURCES)/src/include/R_ext/Parse.h \
	$(R_SOURCES)/src/include/R_ext/Print.h \
	$(R_SOURCES)/src/include/R_ext/PrtUtil.h \
	$(R_SOURCES)/src/include/R_ext/RConverters.h \
	$(R_SOURCES)/src/include/R_ext/Rdynload.h \
	$(R_SOURCES)/src/include/R_ext/Utils.h	\
	$(R_SOURCES)/src/include/R_ext/libextern.h

include_maindir = ${includedir}/main
nodist_include_main_HEADERS = \
	$(R_SOURCES)/src/main/arithmetic.h

librcc_la_SOURCES = \
	rcc_lib.c \
	replacements.c

librcc_la_CFLAGS  = $(BASE_CFLAGS) -I$(R_SOURCES)/src/include -I$(R_SOURCES)/src/main -I.

