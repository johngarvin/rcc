MY_INCLUDES =               \
  -I@R_SOURCES@/src/include \
  -I@OA_PREFIX@/include     \
  -I$(top_srcdir)/src   

bin_PROGRAMS = tra

tra_SOURCES =                             \
  TestHarness.cc                          \
  IRInterface.cc IRInterface.h	          \
  Analyst.cc Analyst.h                    \
  UseDefSolver.cc UseDefSolver.h	  \
  Utils.cc R_Utils.h                      \
  SimpleIterators.cc SimpleIterators.h

tra_CXXFLAGS = -DOA_IRHANDLETYPE_UL $(MY_INCLUDES)

MY_OA_LIBS = @OA_PREFIX@/lib/libOAul.a 

MY_TRA_LIBS =		        \
  $(MY_OA_LIBS)		        \
  -L@R_LIB_PATH@ -lR		\
  -lrcca			\
  -L../annotations -lannot	\
  -L../support -lsupport        \
  -L../matmul -lmatmul

tra_LDADD = $(MY_TRA_LIBS) 

lib_LTLIBRARIES = librcca.la

librcca_la_SOURCES =                       \
  IRInterface.cc IRInterface.h             \
  Analyst.cc Analyst.h                     \
  UseDefSolver.cc UseDefSolver.h	   \
  Utils.cc Utils.h                         \
  SimpleIterators.cc SimpleIterators.h

librcca_la_CXXFLAGS = -DOA_IRHANDLETYPE_UL $(MY_INCLUDES)
