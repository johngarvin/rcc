SUBDIRS = macros support annotations analysis
bin_PROGRAMS = rcc.bin

rcc_bin_SOURCES = \
	      rcc.cc rcc.h \
		  codegen.cc codegen.h \
		  visibility.cc visibility.h \
          Macro.cpp Macro.hpp \
	      get_name.c get_name.h 

#--------------------------------------
# The use of variables defined by -D below in MY_COMMON_INCLUDES 
# within rcc.cc files makes it necessary that its object file 
# depend on the Makefile 
#--------------------------------------
rcc_bin-rcc.$(OBJEXT): Makefile


MY_COMMON_INCLUDES = -I$(R_SOURCES)/src/include -I$(R_HOME)/include -I$(top_srcdir)/lib -DRCC_MACRO_PATH=\"@RCC_MACRO_PATH@\" -DR_HOME=\"@R_HOME@\" -DRCC_INCLUDE_PATH=\"@RCC_INCLUDE_PATH@\" -I./support

rcc_bin_CFLAGS  = $(BASE_CFLAGS) $(MY_COMMON_INCLUDES)
rcc_bin_CXXFLAGS= $(BASE_CXXFLAGS) $(MY_COMMON_INCLUDES)
#  rcc_bin_LDFLAGS =-L$(R_PREFIX)/lib -lR
rcc_bin_LDADD = -L@R_LIB_PATH@ -lR -Lsupport -lsupport -Lannotations -lannot
