libmatmul_la_SOURCES =                 \
  TransformMatMul.h TransformMatMul.cc \
  TransposeMatMul.c  

MY_INCLUDES =        \
  -I@R_SOURCES@/src/include \
  -I$(top_srcdir)/src

lib_LTLIBRARIES = libmatmul.la

libmatmul_la_LIBADD = -L@R_LIB_PATH@ -lR

INCLUDES = $(MY_INCLUDES)

