/*! \file
  
  \brief Declarations of the AnnotationManager that generates an 
         InterSideEffectStandard.

  \authors Michelle Strout, Andy Stone (alias tag update)
  \version $Id: ManagerInterSideEffectStandard.hpp,v 1.5 2005/06/10 02:32:05 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerInterSideEffectStandard_h
#define ManagerInterSideEffectStandard_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/InterSideEffectIRInterface.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectStandard.hpp>
#include <OpenAnalysis/SideEffect/ManagerSideEffectStandard.hpp>
#include <OpenAnalysis/Alias/InterAliasInterface.hpp>
#include <OpenAnalysis/DataFlow/CountDFSet.hpp>
#include <OpenAnalysis/DataFlow/ParamBindings.hpp>
#include <OpenAnalysis/DataFlow/CallGraphDFProblem.hpp>
#include <OpenAnalysis/DataFlow/CallGraphDFSolver.hpp>


namespace OA {
  namespace SideEffect {

/*! 
   Standard implementation of side-effects analysis.  This cass builds an
   InterSideEffectsStandard object to store results.
*/
class ManagerInterSideEffectStandard : private DataFlow::CallGraphDFProblem {
public:
  ManagerInterSideEffectStandard(OA_ptr<InterSideEffectIRInterface> _ir);
  ~ManagerInterSideEffectStandard () {}

  /*! Perform interprocedural side-effects analysis given a call graph
      and alias results */
  OA_ptr<InterSideEffectStandard> 
  performAnalysis( OA_ptr<CallGraph::CallGraphInterface>  callGraph,
                   OA_ptr<DataFlow::ParamBindings>  paramBind,
                   OA_ptr<Alias::InterAliasInterface>  interAlias,
                   OA_ptr<ManagerSideEffectStandard>  intraMan,
                   DataFlow::DFPImplement  algorithm);

private:
  //========================================================
  // implementation of CallGraphDFProblemNew callbacks
  //========================================================
  //--------------------------------------------------------
  // initialization callbacks
  //--------------------------------------------------------
  
  OA_ptr<DataFlow::DataFlowSet>  initializeTop();
  OA_ptr<DataFlow::DataFlowSet>  initializeBottom();
  OA_ptr<DataFlow::DataFlowSet>  initializeNode(ProcHandle proc);

  //! Should generate an initial DataFlowSet, use if for a call if both caller
  //! and callee are defined
  OA_ptr<DataFlow::DataFlowSet>  initializeNodeIN(
    OA_ptr<CallGraph::NodeInterface> n);

  OA_ptr<DataFlow::DataFlowSet>  initializeNodeOUT(
    OA_ptr<CallGraph::NodeInterface> n);

  OA_ptr<DataFlow::DataFlowSet>  initializeEdge(
    CallHandle call, ProcHandle caller, ProcHandle callee);

  //! Should generate an initial DataFlowSet for a call,
  //! called when callee is not defined in call graph and therefore
  //! doesn't have a procedure definition handle
  OA_ptr<DataFlow::DataFlowSet>  initializeEdge(
    CallHandle call, ProcHandle caller, SymHandle callee);


  //--------------------------------------------------------
  // solver callbacks 
  //--------------------------------------------------------
  
  OA_ptr<DataFlow::DataFlowSet>  meet(
    OA_ptr<DataFlow::DataFlowSet> set1, OA_ptr<DataFlow::DataFlowSet> set2); 

  OA_ptr<DataFlow::DataFlowSet>  atCallGraphNode(
    OA_ptr<DataFlow::DataFlowSet> inSet, OA::ProcHandle proc);

  OA_ptr<DataFlow::DataFlowSet>  atCallGraphEdge(
    OA_ptr<DataFlow::DataFlowSet> inSet,
    OA::CallHandle call,
    ProcHandle caller,
    ProcHandle callee);

  OA_ptr<DataFlow::DataFlowSet>  atCallGraphEdge(
    OA_ptr<DataFlow::DataFlowSet> inSet,
    CallHandle call,
    ProcHandle caller,
    SymHandle callee);

  //! translate results from procedure node to callee edge if top-down
  //! or to caller edge if bottom-up
  OA_ptr<DataFlow::DataFlowSet>  nodeToEdge(
    ProcHandle proc, 
    OA_ptr<DataFlow::DataFlowSet> procDFSet,
    CallHandle call)
  {
    return procDFSet;
  }

  //! translate results from caller edge to procedure node if top-down
  //! or from callee edge if bottom-up
  OA_ptr<DataFlow::DataFlowSet>  edgeToNode(
    CallHandle call, 
    OA_ptr<DataFlow::DataFlowSet> callDFSet,
    ProcHandle proc)
  {
    return callDFSet;
  }


private:
  OA_ptr<InterSideEffectIRInterface>  mIR;
  OA_ptr<Alias::InterAliasInterface>  mInterAlias;
  OA_ptr<InterSideEffectStandard>  mInterSideEffect;
  OA_ptr<DataFlow::ParamBindings>  mParamBind;
  OA_ptr<ManagerSideEffectStandard>  mIntraMan;
  OA_ptr<DataFlow::CallGraphDFSolver>  mSolver; 
};

  } // end of SideEffect namespace
} // end of OA namespace

#endif
