SUBDIRS = macros matmul support analysis 
bin_PROGRAMS = rcc.bin

rcc_bin_SOURCES =                \
  Main.cc Main.h                 \
  CodeGen.cc CodeGen.h           \
  CodeGenUtils.cc CodeGenUtils.h \
  Visibility.cc Visibility.h     \
  Macro.cc Macro.h               \
  GetName.cc GetName.h           \
  Output.cc Output.h             \
  ParseInfo.cc ParseInfo.h       \
  CScope.cc CScope.h

#--------------------------------------
# The use of variables defined by -D below in MY_COMMON_MACROS 
# within rcc.cc files makes it necessary that its object file 
# depend on the Makefile 
#--------------------------------------
rcc_bin-rcc.$(OBJEXT): Makefile

R_INCLUDES = -I@R_SOURCES@/src/include

# OA requires OA_IRHANDLETYPE_UL macro to be defined
OA_INCLUDES = \
  -DOA_IRHANDLETYPE_UL      \
  -I@OA_PREFIX@/include

MY_COMMON_INCLUDES =         \
  -I$(top_srcdir)/src        \
  -I$(top_srcdir)/lib        

MY_COMMON_MACROS =                          \
  -DRCC_MACRO_PATH=\"@RCC_MACRO_PATH@\"     \
  -DR_HOME=\"@R_HOME@\"                     \
  -DRCC_INCLUDE_PATH=\"@RCC_INCLUDE_PATH@\"

MY_OA_LIBS = @OA_PREFIX@/lib/libOAul.a 

ALL_INCLUDES = $(R_INCLUDES) $(OA_INCLUDES) $(MY_COMMON_INCLUDES)

rcc_bin_CFLAGS  = $(BASE_CFLAGS) $(ALL_INCLUDES) $(MY_COMMON_MACROS)
rcc_bin_CXXFLAGS= $(BASE_CXXFLAGS) $(ALL_INCLUDES) $(MY_COMMON_MACROS)
rcc_bin_LDADD = -Lanalysis -lrcca -Lsupport -lsupport -L@R_LIB_PATH@ -lR $(MY_OA_LIBS)
