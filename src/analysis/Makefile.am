MY_INCLUDES =               \
  -I@R_SOURCES@/src/include \
  -I@OA_PREFIX@/include     \
  -I$(top_srcdir)/src   

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
  RequiresContext.cc RequiresContext.h    \
  SimpleIterators.cc SimpleIterators.h    \
  VarRef.cc VarRef.h                      \
  VarRefSet.cc VarRefSet.h                \
  HandleInterface.cc HandleInterface.h    \
  LocalVariableAnalysis.cc LocalVariableAnalysis.h \
  LocalFunctionAnalysis.cc LocalFunctionAnalysis.h \
  BindingAnalysis.cc BindingAnalysis.h    \
  Assertion.cc Assertion.h

INCLUDES = $(MY_INCLUDES)

MY_OA_LIBS = @OA_PREFIX@/lib/libOAul.a
AM_CXXFLAGS = -DOA_IRHANDLETYPE_UL 
librcca_la_LDFLAGS = $(MY_OA_LIBS)
