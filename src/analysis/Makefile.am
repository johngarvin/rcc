MY_INCLUDES =               \
  -I@R_SOURCES@/src/include \
  -I@OA_PREFIX@/include     \
  -I$(top_srcdir)/src   

#bin_PROGRAMS = tra

#tra_SOURCES =                             \
#  TestHarness.cc                          

#tra_CXXFLAGS = -DOA_IRHANDLETYPE_UL $(MY_INCLUDES)

MY_OA_LIBS = @OA_PREFIX@/lib/libOAul.a 

#MY_TRA_LIBS =		        \
#  $(MY_OA_LIBS)		        \
#  -L@R_LIB_PATH@ -lR		\
#  -lrcca			\
#  -L../support -lsupport        \
#  -L../matmul -lmatmul

#tra_LDADD = $(MY_TRA_LIBS) 

lib_LTLIBRARIES = librcca.la

librcca_la_SOURCES =                      \
  IRInterface.cc IRInterface.h            \
  Analyst.cc Analyst.h                    \
  UseDefSolver.cc UseDefSolver.h	  \
  ScopeTreeBuilder.cc ScopeTreeBuilder.h  \
  Utils.cc Utils.h                        \
  AnalysisResults.cc  AnalysisResults.h   \
  Annotation.cc  Annotation.h             \
  AnnotationBase.cc  AnnotationBase.h     \
  AnnotationSet.cc   AnnotationSet.h      \
  PropertySet.cc         PropertySet.h    \
  SimpleIterators.cc SimpleIterators.h    \
  VarRef.cc VarRef.h                      \
  VarRefSet.cc VarRefSet.h                \
  HandleInterface.cc HandleInterface.h

INCLUDES = $(MY_INCLUDES)

AM_CXXFLAGS = -DOA_IRHANDLETYPE_UL 
librcca_la_LDFLAGS = $(MY_OA_LIBS)
