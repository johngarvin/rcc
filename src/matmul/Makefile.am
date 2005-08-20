libmatmul_la_SOURCES =                 \
  TransformMatMul.h TransformMatMul.cc \
  TransposeMatMul.c  

MY_INCLUDES =        \
  -I@R_HOME@/include \
  -I..               \
  -I../analysis

lib_LTLIBRARIES = libmatmul.la

libmatmul_la_LIBADD = -L@R_LIB_PATH@ -lR

libmatmul_la_CFLAGS = $(MY_INCLUDES)
libmatmul_la_CXXFLAGS = $(libmatmul_la_CFLAGS)
