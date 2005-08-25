SUBDIRS = analysis annotations macros matmul support
bin_PROGRAMS = rcc.bin

rcc_bin_SOURCES =                \
  Main.cc Main.h                 \
  CodeGen.cc CodeGen.h           \
  CodeGenUtils.cc CodeGenUtils.h \
  Visibility.cc Visibility.h     \
  Macro.cc Macro.h               \
  GetName.c GetName.h            \
  Output.cc Output.h

#--------------------------------------
# The use of variables defined by -D below in MY_COMMON_MACROS 
# within rcc.cc files makes it necessary that its object file 
# depend on the Makefile 
#--------------------------------------
rcc_bin-rcc.$(OBJEXT): Makefile

# below: OA_IRHANDLETYPE_UL macro is required for OA

MY_COMMON_INCLUDES =         \
  -I$(R_SOURCES)/src/include \
  -I$(R_HOME)/include        \
  -DOA_IRHANDLETYPE_UL       \
  -I$(OA_PREFIX)/include     \
  -I$(top_srcdir)/lib        \
  -I./support                \
  -I./analysis               \
  -I./annotations

MY_COMMON_MACROS =                          \
  -DRCC_MACRO_PATH=\"@RCC_MACRO_PATH@\"     \
  -DR_HOME=\"@R_HOME@\"                     \
  -DRCC_INCLUDE_PATH=\"@RCC_INCLUDE_PATH@\"

MY_OA_LIBS = @OA_PREFIX@/lib/libOAul.a 

rcc_bin_CFLAGS  = $(BASE_CFLAGS) $(MY_COMMON_INCLUDES) $(MY_COMMON_MACROS)
rcc_bin_CXXFLAGS= $(BASE_CXXFLAGS) $(MY_COMMON_INCLUDES) $(MY_COMMON_MACROS)
rcc_bin_LDADD = -L@R_LIB_PATH@ -lR -Lsupport -lsupport -Lannotations -lannot -Lanalysis -lrcca $(MY_OA_LIBS)
