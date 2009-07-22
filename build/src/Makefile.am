# SUBDIRS = macros matmul support analysis
bin_PROGRAMS = rcc.bin

VPATH =						\
  @top_srcdir@/src				\
  @top_srcdir@/src/analysis			\
  @top_srcdir@/src/codegen			\
  @top_srcdir@/src/codegen/SubexpBuffer		\
  @top_srcdir@/src/matmul			\
  @top_srcdir@/src/support			\
  @top_srcdir@/src/support/iterators		\
  @top_srcdir@/src/support/stacks		\
  @top_srcdir@/src/support/trees

#  @top_srcdir@/src/analysis/call-graph          \
#


rcc_bin_SOURCES =				\
  Main.cc Main.h				\
  GetName.cc GetName.h				\
  CodeGen.cc CodeGen.h				\
  CodeGenUtils.cc CodeGenUtils.h		\
  LoopContext.cc LoopContext.h			\
  Visibility.cc Visibility.h			\
  Macro.cc Macro.h				\
  Output.cc Output.h				\
  ParseInfo.cc ParseInfo.h			\
  CScope.cc CScope.h				\
  CheckProtect.h				\
  ArgMatcher.cc ArgMatcher.h                    \
						\
  AnalysisException.cc                          \
  AnalysisException.h                           \
  AnalysisResults.cc				\
  AnalysisResults.h				\
  Analyst.cc					\
  Analyst.h					\
  AnnotationBase.cc				\
  AnnotationBase.h				\
  AnnotationMap.cc				\
  AnnotationMap.h				\
  Assertion.cc					\
  Assertion.h					\
  CallByValueAnalysis.cc                        \
  CallByValueAnalysis.h                         \
  CommandLineArgs.cc                            \
  CommandLineArgs.h                             \
  Debug.cc                                      \
  Debug.h                                       \
  DebutDFSolver.cc                              \
  DebutDFSolver.h                               \
  DefaultAnnotationMap.cc                       \
  DefaultAnnotationMap.h                        \
  DefaultDFSet.cc                               \
  DefaultDFSet.h                                \
  DefVar.cc                                     \
  DefVar.h                                      \
  Dependence.h                                  \
  EscapeInfo.h                                  \
  EscapeInfo.cc                                 \
  EscapeInfoAnnotationMap.cc                    \
  EscapeInfoAnnotationMap.h                     \
  ExpressionInfo.cc                             \
  ExpressionInfo.h                              \
  ExpressionInfoAnnotationMap.cc                \
  ExpressionInfoAnnotationMap.h                 \
  ExprTreeBuilder.cc                            \
  ExprTreeBuilder.h                             \
  FormalArgInfo.cc                              \
  FormalArgInfo.h                               \
  FormalArgInfoAnnotationMap.cc                 \
  FormalArgInfoAnnotationMap.h                  \
  FuncInfo.cc                                   \
  FuncInfo.h                                    \
  FuncInfoAnnotationMap.cc                      \
  FuncInfoAnnotationMap.h                       \
  HandleInterface.cc				\
  HandleInterface.h				\
  HellProcedure.cc                              \
  HellProcedure.h                               \
  IRInterface.cc				\
  IRInterface.h					\
  LexicalContext.h                              \
  LexicalContext.cc                             \
  LexicalScope.cc                               \
  LexicalScope.h                                \
  LocalFunctionAnalysis.cc			\
  LocalFunctionAnalysis.h			\
  LocalityDFSet.cc				\
  LocalityDFSetElement.cc			\
  LocalityDFSetElement.h			\
  LocalityDFSet.h				\
  LocalityDFSetIterator.cc			\
  LocalityDFSetIterator.h			\
  LocalityDFSolver.cc				\
  LocalityDFSolver.h				\
  LocalityType.cc                               \
  LocalityType.h				\
  LocalVariableAnalysis.cc			\
  LocalVariableAnalysis.h			\
  MemRefExprInterface.cc                        \
  MemRefExprInterface.h                         \
  Metrics.cc                                    \
  Metrics.h                                     \
  NameMentionMultiMap.h                         \
  NameStmtMultiMap.h                            \
  OACallGraphAnnotation.cc                      \
  OACallGraphAnnotation.h                       \
  OACallGraphAnnotationMap.cc                   \
  OACallGraphAnnotationMap.h                    \
  PropertyHndl.h				\
  PropertySet.cc				\
  PropertySet.h					\
  PropertySetMgr.cc				\
  PropertySetMgr.h				\
  RccBasicInterface.cc                          \
  RccBasicInterface.h                           \
  RequiresContext.cc				\
  RequiresContext.h				\
  ScopeAnnotationMap.cc                         \
  ScopeAnnotationMap.h                          \
  Settings.cc                                   \
  Settings.h                                    \
  SideEffect.cc                                 \
  SideEffect.h                                  \
  SideEffectAnnotationMap.cc                    \
  SideEffectAnnotationMap.h                     \
  SimpleIterators.cc				\
  SimpleIterators.h				\
  StrictnessDFSet.cc                            \
  StrictnessDFSet.h                             \
  StrictnessDFSetElement.cc                     \
  StrictnessDFSetElement.h                      \
  StrictnessDFSetIterator.cc                    \
  StrictnessDFSetIterator.h                     \
  StrictnessDFSolver.cc                         \
  StrictnessDFSolver.h                          \
  StrictnessResult.cc                           \
  StrictnessResult.h                            \
  StrictnessType.cc                             \
  StrictnessType.h                              \
  SymbolTable.cc                                \
  SymbolTable.h                                 \
  SymbolTableFacade.cc                          \
  SymbolTableFacade.h                           \
  TypeInference.h				\
  UseVar.cc                                     \
  UseVar.h                                      \
  Utils.cc					\
  Utils.h					\
  Var.cc                                        \
  Var.h                                         \
  VarInfo.cc                                    \
  VarInfo.h                                     \
  VarAnnotationMap.cc                           \
  VarAnnotationMap.h                            \
  VarBinding.cc                                 \
  VarBinding.h                                  \
  VarBindingAnnotationMap.cc                    \
  VarBindingAnnotationMap.h                     \
  VarRef.cc					\
  VarRef.h					\
  VarRefFactory.cc                              \
  VarRefFactory.h                               \
  VarRefSet.cc					\
  VarRefSet.h					\
  VarVisitor.h                                  \
                                                \
  SplitSubexpBuffer.cc SplitSubexpBuffer.h	\
  SubexpBuffer.cc SubexpBuffer.h		\
  SubexpBufferUtils.cc				\
  Expression.cc Expression.h			\
  new_location.cc                               \
  op_begin.cc					\
  op_break.cc					\
  op_builtin.cc					\
  op_clos_app.cc				\
  op_closure.cc					\
  op_exp.cc					\
  op_for.cc					\
  op_for_colon.cc                               \
  op_fundef.cc					\
  op_if.cc					\
  op_lang.cc					\
  op_list.cc					\
  op_literal.cc					\
  op_primsxp.cc					\
  op_program.cc                                 \
  op_promise.cc					\
  op_return.cc					\
  op_set.cc					\
  op_special.cc					\
  op_string.cc					\
  op_subscriptset.cc				\
  op_var_use.cc					\
  op_var_def.cc					\
  op_vector.cc					\
  op_while.cc					\
						\
  TransformMatMul.cc				\
  TransformMatMul.h				\
  TransposeMatMul.c				\
						\
  Parser.cc Parser.h				\
  RccError.cc RccError.h                        \
  StringUtils.cc StringUtils.h			\
						\
  IteratorStack.cc				\
  StackableIterator.cc				\
						\
  StackTmpl.h					\
  PointerStack.C PointerStack.h			\
						\
  NonUniformDegreeTree.cc			\
  NonUniformDegreeTree.h			\
  NonUniformDegreeTreeTmpl.h

# RCC home-grown call graph
# Commented out because we are using OpenAnalysis call graphs instead

#  RccCallGraphAnnotation.cc                     \
#  RccCallGraphAnnotation.h                      \
#  RccCallGraphAnnotationMap.cc                  \
#  RccCallGraphAnnotationMap.h                   \
#  CallGraphEdge.cc                              \
#  CallGraphEdge.h                               \
#  CallGraphInfo.cc                              \
#  CallGraphInfo.h                               \
#  CallGraphNode.cc                              \
#  CallGraphNode.h                               \
#  CallSiteCallGraphNode.cc                      \
#  CallSiteCallGraphNode.h                       \
#  CoordinateCallGraphNode.cc                    \
#  CoordinateCallGraphNode.h                     \
#  FundefCallGraphNode.cc                        \
#  FundefCallGraphNode.h                         \
#  LibraryCallGraphNode.cc                       \
#  LibraryCallGraphNode.h                        \
#  UnknownValueCallGraphNode.cc                  \
#  UnknownValueCallGraphNode.h                   \
#						


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

AM_CXXFLAGS = @HOST_CXXFLAGS@ $(MY_COMMON_MACROS)

rcc_bin_LDADD = -L@R_LIB_PATH@ -lR $(MY_OA_LIBS)
