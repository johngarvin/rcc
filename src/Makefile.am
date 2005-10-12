SUBDIRS = macros matmul support analysis type-inference
bin_PROGRAMS = rcc.bin

VPATH =                                 \
  @top_srcdir@/src                      \
  @top_srcdir@/src/codegen              \
  @top_srcdir@/src/codegen/SubexpBuffer 

#  @top_srcdir@/src/matmul               
#  @top_srcdir@/src/support              
#  @top_srcdir@/src/analysis             


rcc_bin_SOURCES =				\
  Main.cc Main.h				\
  CodeGen.cc CodeGen.h				\
  CodeGenUtils.cc CodeGenUtils.h		\
  LoopContext.cc LoopContext.h			\
  Visibility.cc Visibility.h			\
  Macro.cc Macro.h				\
  GetName.cc GetName.h				\
  Output.cc Output.h				\
  ParseInfo.cc ParseInfo.h			\
  CScope.cc CScope.h				\
						\
  SplitSubexpBuffer.cc SplitSubexpBuffer.h	\
  SubexpBuffer.cc SubexpBuffer.h		\
  SubexpBufferUtils.cc				\
  Expression.cc Expression.h			\
  op_begin.cc					\
  op_break.cc					\
  op_builtin.cc					\
  op_clos_app.cc				\
  op_closure.cc					\
  op_exp.cc					\
  op_for.cc					\
  op_fundef.cc					\
  op_if.cc					\
  op_lang.cc					\
  op_list.cc					\
  op_literal.cc					\
  op_primsxp.cc					\
  op_promise.cc					\
  op_return.cc					\
  op_set.cc					\
  op_special.cc					\
  op_string.cc					\
  op_subscriptset.cc				\
  op_var_use.cc					\
  op_var_def.cc					\
  op_vector.cc					\
  op_while.cc


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
  -I@top_srcdir@/src        \
  -I@top_srcdir@/lib

MY_COMMON_MACROS =                          \
  -DRCC_MACRO_PATH=\"@RCC_MACRO_PATH@\"     \
  -DR_HOME=\"@R_HOME@\"                     \
  -DRCC_INCLUDE_PATH=\"@RCC_INCLUDE_PATH@\"

MY_OA_LIBS = @OA_PREFIX@/lib/libOAul.a 

ALL_INCLUDES = $(R_INCLUDES) $(OA_INCLUDES) $(MY_COMMON_INCLUDES)

INCLUDES = $(ALL_INCLUDES)

AM_CXXFLAGS = $(MY_COMMON_MACROS)

rcc_bin_LDADD = -Lanalysis -lrcca -Lsupport -lsupport -L@R_LIB_PATH@ -lR $(MY_OA_LIBS)
