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

librcca_la_SOURCES =                       \
  IRInterface.cc IRInterface.h             \
  Analyst.cc Analyst.h                     \
  UseDefSolver.cc UseDefSolver.h	   \
  ScopeTree.h                 \
  Utils.cc Utils.h                         \
  Annotation.cc  Annotation.h  \
  AnnotationBase.cc  AnnotationBase.h \
  AnnotationSet.cc   AnnotationSet.h      \
  PropertySet.cc         PropertySet.h \
  PropertySetMgr.cc PropertySetMgr.h \
  SimpleIterators.cc SimpleIterators.h \
  VarRef.cc VarRef.h \
  VarRefSet.cc VarRefSet.h

librcca_la_CXXFLAGS = -DOA_IRHANDLETYPE_UL $(MY_INCLUDES)
