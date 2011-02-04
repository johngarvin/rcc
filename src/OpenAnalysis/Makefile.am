# -*-Mode: makefile;-*-
# $Header: /Volumes/cvsrep/developer/OpenAnalysis/src/OpenAnalysis/Makefile.am,v 1.8.6.5 2006/01/19 05:30:47 mstrout Exp $

## * BeginRiceCopyright *****************************************************
## 
## Copyright ((c)) 2002, Rice University 
## All rights reserved.
## 
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are
## met:
## 
## * Redistributions of source code must retain the above copyright
##   notice, this list of conditions and the following disclaimer.
## 
## * Redistributions in binary form must reproduce the above copyright
##   notice, this list of conditions and the following disclaimer in the
##   documentation and/or other materials provided with the distribution.
## 
## * Neither the name of Rice University (RICE) nor the names of its
##   contributors may be used to endorse or promote products derived from
##   this software without specific prior written permission.
## 
## This software is provided by RICE and contributors "as is" and any
## express or implied warranties, including, but not limited to, the
## implied warranties of merchantability and fitness for a particular
## purpose are disclaimed. In no event shall RICE or contributors be
## liable for any direct, indirect, incidental, special, exemplary, or
## consequential damages (including, but not limited to, procurement of
## substitute goods or services; loss of use, data, or profits; or
## business interruption) however caused and on any theory of liability,
## whether in contract, strict liability, or tort (including negligence
## or otherwise) arising in any way out of the use of this software, even
## if advised of the possibility of such damage. 
## 
## ******************************************************* EndRiceCopyright *
##
## Copyright (c) 2006 Contributors

#############################################################################
#
# File:
#   $Source: /Volumes/cvsrep/developer/OpenAnalysis/src/OpenAnalysis/Makefile.am,v $
#
# Description:
#   *Process with automake to produce Makefile.in*
#
#   Note: All local variables are prefixed with MY to prevent name
#   clashes with automatic automake variables.
#
#############################################################################

# We do not want the standard GNU files (NEWS README AUTHORS ChangeLog...)
AUTOMAKE_OPTIONS = foreign

#############################################################################
# Common settings
#############################################################################

include $(top_srcdir)/src/Makeinclude.config

#############################################################################
# Local settings
#############################################################################

# Re-added SSA for R analysis --garvin

# Order: Interface; engines in alphabetical order; utils
MYINSTALLEDHEADERS = \
  IRInterface/AliasIRInterface.hpp \
  IRInterface/IRHandles.hpp \
  IRInterface/CallGraphDFProblemIRInterface.hpp \
  IRInterface/CFGIRInterface.hpp \
  IRInterface/CFGIRInterfaceDefault.hpp \
  IRInterface/ConstValBasicInterface.hpp \
  IRInterface/ConstValIntInterface.hpp \
  IRInterface/EvalToConstVisitorIRInterface.hpp \
  IRInterface/ExprTreeIRInterface.hpp \
  IRInterface/OpBasicInterface.hpp \
  IRInterface/SideEffectIRInterface.hpp \
  IRInterface/InterSideEffectIRInterface.hpp \
  IRInterface/InterSideEffectIRInterfaceDefault.hpp \
  IRInterface/LivenessIRInterface.hpp \
  IRInterface/CallGraphIRInterface.hpp \
  IRInterface/ICFGIRInterface.hpp \
  IRInterface/OpBasicInterface.hpp \
  IRInterface/ParamBindingsIRInterface.hpp \
  IRInterface/ReachDefsIRInterface.hpp \
  IRInterface/ActivityIRInterface.hpp \
  IRInterface/AssignPairIterator.hpp \
  IRInterface/ReachConstsIRInterface.hpp \
  IRInterface/UDDUChainsIRInterface.hpp \
  IRInterface/auto_LivenessIRInterface.hpp \
  IRInterface/auto_ReachingDefsIRInterface.hpp \
  IRInterface/auto_AvailExprIRInterface.hpp \
  IRInterface/auto_VaryIRInterface.hpp \
  IRInterface/DUGIRInterface.hpp \
  \
  Activity/UsefulStandard.hpp \
  Activity/ManagerICFGUseful.hpp \
  Activity/InterUseful.hpp \
  Activity/ActivePerStmt.hpp \
  Activity/InterVary.hpp \
  Activity/VaryStandard.hpp \
  Activity/ManagerICFGVaryActive.hpp \
  Activity/ActiveStandard.hpp \
  Activity/InterActiveFortran.hpp \
  Activity/InterActive.hpp \
  Activity/DepDFSet.hpp \
  Activity/ManagerICFGActive.hpp \
  \
  OABase/Annotation.hpp \
  \
  Alias/AliasTag.hpp \
  Alias/AliasTagSet.hpp \
  Alias/InterAliasInterface.hpp \
  Alias/ManagerFIAlias.hpp \
  Alias/Interface.hpp \
  Alias/ManagerFIAliasAliasTag.hpp \
  Alias/AliasTagResults.hpp \
  Alias/NotationGenerator.hpp \
  \
  CFG/ManagerCFG.hpp \
  CFG/CFG.hpp \
  CFG/CFGInterface.hpp \
  CFG/EachCFGInterface.hpp \
  CFG/EachCFGStandard.hpp \
  \
  CallGraph/CallGraphInterface.hpp \
  CallGraph/CallGraph.hpp \
  CallGraph/ManagerCallGraph.hpp \
  \
  ICFG/ICFGInterface.hpp \
  ICFG/ICFG.hpp \
  ICFG/ManagerICFG.hpp \
  \
  DataFlow/CallGraphDFProblem.hpp \
  DataFlow/CallGraphDFSolver.hpp \
  DataFlow/CFGDFProblem.hpp \
  DataFlow/CFGDFSolver.hpp \
  DataFlow/CountDFSet.hpp \
  DataFlow/DataFlowSet.hpp \
  DataFlow/DataFlowSetImpl.hpp \
  DataFlow/DFSetBitImpl.hpp \
  DataFlow/DGraphSolverDFP.hpp \
  DataFlow/ICFGDFProblem.hpp \
  DataFlow/ICFGDFSolver.hpp \
  DataFlow/IRHandleDataFlowSet.hpp \
  DataFlow/WorkList.hpp \
  DataFlow/WorkListPQueue.hpp \
  DataFlow/WorkListQueue.hpp \
  DataFlow/ParamBindings.hpp \
  DataFlow/ManagerParamBindings.hpp \
  \
  DFAGen/Liveness/auto_Liveness.hpp \
  DFAGen/Liveness/auto_ManagerLiveness.hpp \
  \
  DFAGen/ReachingDefs/auto_ReachingDefs.hpp \
  DFAGen/ReachingDefs/auto_ManagerReachingDefs.hpp \
  \
  DFAGen/Vary/auto_Vary.hpp \
  DFAGen/Vary/auto_ManagerVary.hpp \
  \
  ExprTree/ExprTree.hpp \
  ExprTree/ExprTreeVisitor.hpp \
  ExprTree/OutputExprTreeVisitor.hpp \
  ExprTree/CollectMREVisitor.hpp \
  ExprTree/EvalToConstVisitor.hpp \
  \
  Liveness/LivenessStandard.hpp \
  Liveness/ManagerLivenessStandard.hpp \
  Liveness/Interface.hpp \
  \
  MemRefExpr/MemRefExpr.hpp \
  MemRefExpr/MemRefExprVisitor.hpp \
  \
  ReachConsts/Interface.hpp \
  ReachConsts/RCPair.hpp \
  ReachConsts/RCPairSet.hpp \
  ReachConsts/ReachConsts.hpp \
  ReachConsts/InterReachConsts.hpp \
  ReachConsts/ManagerICFGReachConsts.hpp \
  \
  ReachDefs/Interface.hpp \
  ReachDefs/ReachDefsStandard.hpp \
  ReachDefs/ManagerReachDefsStandard.hpp \
  \
  ReachDefsOverwrite/Interface.hpp \
  ReachDefsOverwrite/ReachDefsOverwriteStandard.hpp \
  ReachDefsOverwrite/ManagerReachDefsOverwriteStandard.hpp \
  \
  SideEffect/SideEffectInterface.hpp \
  SideEffect/SideEffectStandard.hpp \
  SideEffect/ManagerSideEffectStandard.hpp \
  SideEffect/ManagerInterSideEffectStandard.hpp \
  SideEffect/InterSideEffectInterface.hpp \
  SideEffect/InterSideEffectStandard.hpp \
  \
  SSA/SSAStandard.hpp \
  SSA/ManagerSSAStandard.hpp \
  SSA/Phi.hpp \
  \
  UDDUChains/Interface.hpp \
  UDDUChains/UDDUChainsStandard.hpp \
  UDDUChains/ManagerUDDUChainsStandard.hpp \
  \
  Utils/DomTree.hpp \
  Utils/Exception.hpp \
  Utils/Iterator.hpp \
  Utils/SetIterator.hpp \
  Utils/ListIterator.hpp \
  Utils/GenOutputTool.hpp \
  Utils/OA_ptr.hpp \
  Utils/OutputBuilder.hpp \
  Utils/OutputBuilderText.hpp \
  Utils/OutputBuilderDOT.hpp \
  Utils/Tree.hpp \
  Utils/UnionFindUniverse.hpp \
  Utils/Util.hpp \
  Utils/DGraph/DGraphInterface.hpp \
  Utils/DGraph/DGraphImplement.hpp \
  Utils/RIFG.hpp \
  Utils/NestedSCR.hpp 


#  IRInterface/CalleeToCallerVisitorIRInterface.hpp \
#  IRInterface/AffineExprIRInterface.hpp \
#  IRInterface/AliasIRInterfaceDefault.hpp \
#  IRInterface/DataDepIRInterface.hpp \
#  IRInterface/ICFGIRInterface.hpp \
#  IRInterface/ActivityIRInterface.hpp \
#  IRInterface/AssignPairIterator.hpp \
#  IRInterface/ParamBindingsIRInterface.hpp \
#  IRInterface/ReachConstsIRInterface.hpp \
#  IRInterface/SSAIRInterface.hpp \
#  IRInterface/UDDUChainsIRInterface.hpp \
#  IRInterface/XAIFIRInterface.hpp \
#  IRInterface/LinearityIRInterface.hpp \
#  IRInterface/LivenessIRInterface.hpp \
#  IRInterface/auto_ReachingDefsIRInterface.hpp \
#  IRInterface/auto_AvailableExpressions.hpp \
#  IRInterface/DUGIRInterface.hpp \
#  IRInterface/LoopIRInterface.hpp \
#  \
#  Activity/DepDFSet.hpp \
#  Activity/ManagerDepStandard.hpp \
#  Activity/DepStandard.hpp \
#  Activity/ManagerInterDep.hpp \
#  Activity/InterDep.hpp \
#  Activity/ManagerICFGDep.hpp \
#  Activity/ICFGDep.hpp \
#  Activity/VaryStandard.hpp \
#  Activity/ManagerICFGVaryActive.hpp \
#  Activity/ManagerICFGUseful.hpp \
#  Activity/InterVary.hpp \
#  Activity/ManagerUsefulStandard.hpp \
#  Activity/UsefulStandard.hpp \
#  Activity/InterUseful.hpp \
#  Activity/ManagerActiveStandard.hpp \
#  Activity/ActiveStandard.hpp \
#  Activity/ActivePerStmt.hpp \
#  Activity/ManagerEachActive.hpp \
#  Activity/ManagerInterActive.hpp \
#  Activity/ManagerICFGActive.hpp \
#  Activity/InterActive.hpp \
#  Activity/InterActiveFortran.hpp \
#  \
#  AffineExpr/AffineExprAbstraction.hpp \
#  AffineExpr/ManagerAffineExpr.hpp \
#  \
#  Alias/Interface.hpp \
#  Alias/EquivSets.hpp \
#  Alias/EquivSets.hpp \
#  Alias/ManagerFIAlias.hpp \
#  Alias/ManagerFIAliasEquivSets.hpp \
#  Alias/ManagerFIAliasAliasMap.hpp \
#  Alias/AliasMap.hpp \
#  Alias/ManagerAliasMapBasic.hpp \
#  Alias/InterAliasMap.hpp \
#  Alias/ManagerInterAliasMapBasic.hpp \
#  Alias/NotationGenerator.hpp \
#  \
#  DataDep/DataDepResults.hpp \
#  DataDep/Interface.hpp \
#  DataDep/ManagerDataDepGCD.hpp \
#  DataDep/ManagerDataDep.hpp \
#  \
#  DataFlow/DFAGenDFSet.hpp \
#  DataFlow/WorkList.hpp \
#  DataFlow/WorkListPQueue.hpp \
#  DataFlow/WorkListQueue.hpp \
#  DataFlow/CFGDFProblem.hpp \
#  DataFlow/CFGDFSolver.hpp \
#  DataFlow/CFGDFSolver.hpp \
#  DataFlow/ManagerParamBindings.hpp \
#  DataFlow/ParamBindings.hpp \
#  DataFlow/CalleeToCallerVisitor.hpp \
#  DataFlow/LocDFSet.hpp \
#  DataFlow/ICFGDFProblem.hpp \
#  DataFlow/ICFGDFSolver.hpp \
#  \
#  DFAGen/ReachingDefs/auto_ReachingDefsStandard.hpp \
#  DFAGen/ReachingDefs/auto_ManagerReachingDefsStandard.hpp \
#  \
#  ExprTree/EvalToMemRefVisitor.hpp \
#  \
#  ICFG/ICFGInterface.hpp \
#  ICFG/ICFG.hpp \
#  ICFG/ManagerICFG.hpp \
#  \
#  CSFIActivity/DUGInterface.hpp \
#  CSFIActivity/DUGStandard.hpp \
#  CSFIActivity/ManagerDUGStandard.hpp \
#  CSFIActivity/ManagerDUActive.hpp \
#  \
#  Linearity/Interface.hpp \
#  Linearity/LinearityPair.hpp \
#  Linearity/LinearityDepsSet.hpp \
#  Linearity/LinearityMatrixStandard.hpp \
#  Linearity/ManagerLinearityStandard.hpp \
#  \
#  Location/Location.hpp \
#  Location/InvisibleLoc.hpp \
#  Location/LocationVisitor.hpp \
#  Location/LocFieldSubSet.hpp \
#  Location/LocIdxSubSet.hpp \
#  Location/LocSet.hpp \
#  Location/LocSubSet.hpp \
#  Location/NamedLoc.hpp \
#  Location/UnknownLoc.hpp \
#  Location/UnnamedLoc.hpp \
#  Location/Iterators/LocIterator.hpp \
#  Location/Iterators/LocSetIterator.hpp \
#  Location/Iterators/OverlapSymIterator.hpp \
#  Location/Locations.hpp \
#  \
#  Loop/LoopAbstraction.hpp \
#  Loop/LoopIndex.hpp \
#  Loop/LoopManager.hpp \
#  Loop/Interface.hpp \
#  Loop/LoopResults.hpp \
#  \
#  ReachConsts/ReachConstsStandard.hpp \
#  ReachConsts/ManagerReachConstsStandard.hpp \
#  ReachConsts/Interface.hpp\
#  ReachConsts/InterReachConsts.hpp \
#  ReachConsts/ManagerInterReachConsts.hpp \
#  ReachConsts/ManagerICFGReachConsts.hpp \
#  \
#  ReachDefsOverwrite/Interface.hpp \
#  ReachDefsOverwrite/ReachDefsOverwriteStandard.hpp \
#  ReachDefsOverwrite/ManagerReachDefsOverwriteStandard.hpp \
#  \
#  \
#  SSA/SSAStandard.hpp \
#  SSA/ManagerSSAStandard.hpp \
#  SSA/Phi.hpp \
#  \
#  UDDUChains/Interface.hpp \
#  UDDUChains/UDDUChainsStandard.hpp \
#  UDDUChains/ManagerUDDUChainsStandard.hpp \
#  \
#  XAIF/ChainsXAIF.hpp \
#  XAIF/ManagerUDDUChainsXAIF.hpp \
#  XAIF/UDDUChainsXAIF.hpp \
#  XAIF/ManagerAliasMapXAIF.hpp \
#  XAIF/AliasMapXAIF.hpp \
#  XAIF/ManagerReachDefsOverwriteXAIF.hpp \
#  XAIF/ReachDefsOverwriteXAIF.hpp 


# Order: Interface; engines in alphabetical order; utils
MYSOURCES = \
  \
  OABase/Annotation.cpp \
  \
  Utils/OutputBuilder.cpp \
  Utils/OutputBuilderText.cpp \
  Utils/OutputBuilderDOT.cpp \
  \
  Alias/AliasTag.cpp \
  Alias/AliasTagSet.cpp \
  Alias/ManagerFIAlias.cpp \
  Alias/ManagerFIAliasAliasTag.cpp \
  Alias/AliasTagResults.cpp \
  Alias/AliasTagResults_output.cpp \
  Alias/NotationGenerator.cpp \
  \
  Activity/UsefulStandard.cpp \
  Activity/ManagerICFGUseful.cpp \
  Activity/ActivePerStmt.cpp \
  Activity/VaryStandard.cpp \
  Activity/ManagerICFGVaryActive.cpp \
  Activity/ActiveStandard.cpp \
  Activity/InterActiveFortran.cpp \
  Activity/DepDFSet.cpp \
  Activity/ManagerICFGActive.cpp \
  \
  CFG/CFG.cpp \
  CFG/ManagerCFG.cpp \
  \
  CallGraph/CallGraph.cpp \
  CallGraph/ManagerCallGraph.cpp \
  \
  ICFG/ICFG.cpp \
  ICFG/ManagerICFG.cpp \
  \
  DataFlow/CallGraphDFSolver.cpp \
  DataFlow/CountDFSet.cpp \
  DataFlow/CountDFSet_output.cpp \
  DataFlow/CFGDFSolver.cpp \
  DataFlow/DGraphSolverDFP.cpp \
  DataFlow/ICFGDFSolver.cpp \
  DataFlow/ManagerParamBindings.cpp \
  DataFlow/ParamBindings.cpp \
  \
  DFAGen/Liveness/auto_Liveness.cpp \
  DFAGen/Liveness/auto_ManagerLiveness.cpp \
  \
  DFAGen/ReachingDefs/auto_ReachingDefs.cpp \
  DFAGen/ReachingDefs/auto_ManagerReachingDefs.cpp \
  \
  DFAGen/Vary/auto_Vary.cpp \
  DFAGen/Vary/auto_ManagerVary.cpp \
  \
  ExprTree/ExprTree.cpp \
  ExprTree/OutputExprTreeVisitor.cpp \
  ExprTree/EvalToConstVisitor.cpp \
  \
  IRInterface/IRHandles.cpp \
  \
  Liveness/ManagerLivenessStandard.cpp \
  Liveness/LivenessStandard.cpp \
  \
  MemRefExpr/MemRefExpr.cpp \
  \
  ReachConsts/RCPair.cpp \
  ReachConsts/RCPairSet.cpp \
  ReachConsts/ReachConsts.cpp \
  ReachConsts/InterReachConsts.cpp \
  ReachConsts/ManagerICFGReachConsts.cpp \
  \
  ReachDefs/ManagerReachDefsStandard.cpp \
  ReachDefs/ReachDefsStandard.cpp \
  \
  ReachDefsOverwrite/ReachDefsOverwriteStandard.cpp \
  ReachDefsOverwrite/ManagerReachDefsOverwriteStandard.cpp \
  \
  SideEffect/SideEffectStandard.cpp \
  SideEffect/SideEffectStandard_output.cpp \
  SideEffect/ManagerSideEffectStandard.cpp \
  SideEffect/InterSideEffectStandard.cpp \
  SideEffect/InterSideEffectStandard_output.cpp \
  SideEffect/ManagerInterSideEffectStandard.cpp \
  \
  SSA/SSAStandard.cpp \
  SSA/ManagerSSAStandard.cpp \
  SSA/Phi.cpp \
  \
  UDDUChains/UDDUChainsStandard.cpp \
  UDDUChains/ManagerUDDUChainsStandard.cpp \
  \
  Utils/DomTree.cpp \
  Utils/Tree.cpp \
  Utils/UnionFindUniverse.cpp \
  Utils/DGraph/DGraphImplement.cpp \
  Utils/RIFG.cpp \
  Utils/NestedSCR.cpp 


#  AffineExpr/AffineExprAbstraction.cpp \
#  AffineExpr/AffineExprAbstraction_output.cpp \
#  AffineExpr/ManagerAffineExpr.cpp \
#  \
#  Alias/EquivSets.cpp \
#  Alias/EquivSets_output.cpp \
#  Alias/ManagerFIAlias.cpp \
#  Alias/ManagerFIAliasEquivSets.cpp \
#  Alias/ManagerFIAliasAliasMap.cpp \
#  Alias/AliasMap.cpp \
#  Alias/AliasMap_output.cpp \
#  Alias/ManagerAliasMapBasic.cpp \
#  Alias/ManagerInterAliasMapBasic.cpp \
#  Alias/NotationGenerator.cpp \
#  \
#  CallGraph/CallGraph.cpp \
#  CallGraph/ManagerCallGraph.cpp \
#  \
#  DataDep/DataDepResults.cpp \
#  DataDep/DataDepResults_output.cpp \
#  DataDep/ManagerDataDep.cpp \
#  DataDep/ManagerDataDepGCD.cpp \
#  \
#  DataFlow/CFGDFSolver.cpp \
#  DataFlow/LocDFSet.cpp \
#  DataFlow/ManagerParamBindings.cpp \
#  DataFlow/ParamBindings.cpp \
#  DataFlow/ParamBindings_output.cpp \
#  DataFlow/CalleeToCallerVisitor.cpp \
#  DataFlow/ICFGDFSolver.cpp \
#  \
#  DFAGen/ReachingDefs/auto_ReachingDefsStandard.cpp \
#  DFAGen/ReachingDefs/auto_ManagerReachingDefsStandard.cpp \
#  \
#  ICFG/ICFG.cpp \
#  ICFG/ManagerICFG.cpp \
#  \
#  CSFIActivity/DUGStandard.cpp \
#  CSFIActivity/ManagerDUGStandard.cpp \
#  CSFIActivity/ManagerDUActive.cpp \
#  \
#  Linearity/LinearityPair.cpp \
#  Linearity/LinearityDepsSet.cpp \
#  Linearity/LinearityMatrixStandard.cpp \
#  Linearity/ManagerLinearityStandard.cpp \
#  \
#  Location/LocationVisitor.cpp \
#  Location/Location.cpp \
#  Location/InvisibleLoc.cpp \
#  Location/LocFieldSubSet.cpp \
#  Location/LocIdxSubSet.cpp \
#  Location/LocSet.cpp \
#  Location/LocSubSet.cpp \
#  Location/NamedLoc.cpp \
#  Location/UnknownLoc.cpp \
#  Location/UnnamedLoc.cpp \
#  Location/Iterators/LocIterator.cpp \
#  Location/Iterators/LocSetIterator.cpp \
#  Location/Iterators/OverlapSymIterator.cpp \
#  \
#  Loop/LoopAbstraction.cpp \
#  Loop/LoopAbstraction_output.cpp \
#  Loop/LoopIndex.cpp \
#  Loop/LoopIndex_output.cpp \
#  Loop/LoopManager.cpp \
#  Loop/LoopResults.cpp \
#  \
#  ReachConsts/ReachConstsStandard.cpp \
#  ReachConsts/ManagerReachConstsStandard.cpp \
#  ReachConsts/InterReachConsts.cpp \
#  ReachConsts/ManagerInterReachConsts.cpp \
#  ReachConsts/ManagerICFGReachConsts.cpp \
#  \
#  ReachDefsOverwrite/ManagerReachDefsOverwriteStandard.cpp \
#  ReachDefsOverwrite/ReachDefsOverwriteStandard.cpp \
#  \
#  SSA/SSAStandard.cpp \
#  SSA/ManagerSSAStandard.cpp \
#  SSA/Phi.cpp \
#  \
#  UDDUChains/UDDUChainsStandard.cpp \
#  UDDUChains/ManagerUDDUChainsStandard.cpp \
#  \
#  XAIF/ChainsXAIF.cpp \
#  XAIF/UDDUChainsXAIF.cpp \
#  XAIF/ManagerUDDUChainsXAIF.cpp \
#  XAIF/AliasMapXAIF.cpp \
#  XAIF/ManagerAliasMapXAIF.cpp \
#  XAIF/ReachDefsOverwriteXAIF.cpp \
#  XAIF/ManagerReachDefsOverwriteXAIF.cpp



MYCXXFLAGS = @HOST_CXXFLAGS@ $(OA_IFLAGS)

MYLIBADD = @HOST_LIBADD@

if IS_HOST_AR
  MYAR = @HOST_AR@
else
  MYAR = $(AR) cru
endif


#############################################################################
# Automake rules
#############################################################################

#-------------------------------------------------
# Library
#-------------------------------------------------

lib_LIBRARIES =
if OPT_ENABLE_LIBNATIVE
  lib_LIBRARIES += libOAul.a
endif
if OPT_ENABLE_LIBCROSS
  lib_LIBRARIES += libOAsz64.a
endif


if OPT_ENABLE_LIBNATIVE
  libOAul_a_SOURCES = $(MYSOURCES)
  libOAul_a_CXXFLAGS = $(MYCXXFLAGS) -DOA_IRHANDLETYPE_UL 
  libOAul_a_AR = $(MYAR)
  libOAul_a_LIBADD = $(MYLIBADD)
endif

if OPT_ENABLE_LIBCROSS
  libOAsz64_a_SOURCES = $(MYSOURCES)
  libOAsz64_a_CXXFLAGS = $(MYCXXFLAGS) -DOA_IRHANDLETYPE_SZ64
  libOAsz64_a_AR = $(MYAR)
  libOAsz64_a_LIBADD = $(MYLIBADD)
endif

nobase_pkginclude_HEADERS = $(MYINSTALLEDHEADERS)

#-------------------------------------------------
# Test Programs
#-------------------------------------------------

check_PROGRAMS = grtest trtest

trtest_SOURCES = Utils/trtest.cpp 
trtest_CXXFLAGS = $(MYCXXFLAGS) -DOA_IRHANDLETYPE_UL
trtest_LDFLAGS = -L$(builddir) -lOAul

grtest_SOURCES = Utils/grtest.cpp
grtest_CXXFLAGS = $(MYCXXFLAGS) -DOA_IRHANDLETYPE_UL
grtest_LDFLAGS = -L$(builddir) -lOAul 


#############################################################################
# Common rules
#############################################################################

include $(top_srcdir)/src/Makeinclude.rules

# The following have temporarily been removed from the build system:

#  IRInterface/XAIFIRInterface.hpp \
#
#  XAIF/AliasMapXAIF.hpp \
#  XAIF/ManagerAliasMapXAIF.hpp \
#  \
#  XAIF/ChainsXAIF.hpp \
#  XAIF/UDDUChainsXAIF.hpp \
#  XAIF/ManagerUDDUChainsXAIF.hpp \
#  \
#  XAIF/ReachDefsOverwriteXAIF.hpp \
#  XAIF/ManagerReachDefsOverwriteXAIF.hpp \
#  \
#
#  \
#  XAIF/AliasMapXAIF.cpp \
#  XAIF/ManagerAliasMapXAIF.cpp \
#  \
#  XAIF/ChainsXAIF.cpp \
#  XAIF/UDDUChainsXAIF.cpp \
#  XAIF/ManagerUDDUChainsXAIF.cpp \
#  \
#  XAIF/ReachDefsOverwriteXAIF.cpp \
#  XAIF/ManagerReachDefsOverwriteXAIF.cpp \
#
#  CSFIActivity/DUGInterface.hpp \
#  CSFIActivity/DUGStandard.hpp \
#  CSFIActivity/ManagerDUGStandard.hpp \
#  CSFIActivity/ManagerDUActive.hpp \
#  \
#
# \
#  CSFIActivity/DUGStandard.cpp \
#  CSFIActivity/ManagerDUGStandard.cpp \
#  CSFIActivity/ManagerDUActive.cpp \
#
