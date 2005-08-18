MY_INCLUDES =               \
  -I@R_HOME@/include        \
  -I@R_SOURCES@/src/include \
  -I@OA_PREFIX@/include     \
  -I..                      \
  -I../annotations          \
  -I../support

bin_PROGRAMS = tra

tra_SOURCES = TestHarness.cc		        \
  R_IRInterface.cpp R_IRInterface.h		\
  R_Analyst.cpp R_Analyst.h			\
  R_UseDefSolver.cpp R_UseDefSolver.hpp		\
  R_Utils.cpp R_Utils.hpp                       \
  SimpleIterators.cpp SimpleIterators.hpp

tra_CXXFLAGS = -DOA_IRHANDLETYPE_UL $(MY_INCLUDES)

MY_OA_LIBS = @OA_PREFIX@/lib/libOAul.a 

MY_TRA_LIBS =		        \
  $(MY_OA_LIBS)		        \
  -L@R_LIB_PATH@ -lR		\
  -lrcca			\
  -L../annotations -lannot	\
  -L../support -lsupport

tra_LDADD = $(MY_TRA_LIBS) 

lib_LTLIBRARIES = librcca.la

librcca_la_SOURCES =		\
  R_opt.cc R_opt.h		\
  R_IRInterface.h R_IRInterface.cpp 

#librcca_la_LDFLAGS = $(MY_OA_LIBS) 
librcca_la_CXXFLAGS = -DOA_IRHANDLETYPE_UL $(MY_INCLUDES)
