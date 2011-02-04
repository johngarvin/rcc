/*! \file
  
  \brief Declarations of an AnnotationManager that generates an 
         InterReachConsts using the ICFGDFSolver.

  \authors Michelle Strout
  \version $Id: ManagerICFGReachConsts.hpp,v 1.1.2.1 2006/02/10 07:57:31 mstrout Exp $

  Copyright (c) 2002-2004, Rice University <br>
  Copyright (c) 2004, University of Chicago <br>  
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>

*/

#ifndef ManagerICFGReachConsts_H
#define ManagerICFGReachConsts_H

//--------------------------------------------------------------------
#include <cassert>

// OpenAnalysis headers
#include <OpenAnalysis/Alias/InterAliasInterface.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/IRInterface/ReachConstsIRInterface.hpp>

#include <OpenAnalysis/DataFlow/ICFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/ICFGDFSolver.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/DataFlow/ParamBindings.hpp>

#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/ExprTree/EvalToConstVisitor.hpp>
#include <OpenAnalysis/ExprTree/EvalToMemRefVisitor.hpp>
#include <OpenAnalysis/DataFlow/CalleeToCallerVisitor.hpp>
#include <OpenAnalysis/ReachConsts/InterReachConsts.hpp>

namespace OA {
  namespace ReachConsts {

/*! 
    Creates InterReachConsts, which can be queried for a ReachConsts for
    each procedure.  
*/
class ManagerICFGReachConsts : public virtual DataFlow::ICFGDFProblem
{ 
public:
  ManagerICFGReachConsts(OA_ptr<ReachConstsIRInterface> _ir);
  ~ManagerICFGReachConsts () {}

  OA_ptr<InterReachConsts> 
  performAnalysis(OA_ptr<ICFG::ICFGInterface> icfg,
                  OA_ptr<DataFlow::ParamBindings> paramBind,
                  OA_ptr<Alias::InterAliasInterface> interAlias,
                  OA_ptr<SideEffect::InterSideEffectInterface> interSE,
                  DataFlow::DFPImplement algorithm);

private:
  //========================================================
  // implementation of ICFGDFProblem interface
  //========================================================
  //--------------------------------------------------------
  // initialization callbacks
  //--------------------------------------------------------

  //! Return an initialized top set
  OA_ptr<DataFlow::DataFlowSet>  initializeTop();
  
  //! Should generate an in and out DataFlowSet for node
  OA_ptr<DataFlow::DataFlowSet> 
  initializeNodeIN(OA_ptr<ICFG::NodeInterface> n);
  OA_ptr<DataFlow::DataFlowSet> 
      initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n);
 
  //--------------------------------------------------------
  // solver callbacks 
  //--------------------------------------------------------
  
  //! OK to modify set1 and return it as result, because solver
  //! only passes a tempSet in as set1
  OA_ptr<DataFlow::DataFlowSet> meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                                     OA_ptr<DataFlow::DataFlowSet> set2); 

  //! OK to modify in set and return it again as result because
  //! solver clones the BB in sets
  OA_ptr<DataFlow::DataFlowSet> 
      transfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in, 
               OA::StmtHandle stmt);
  
  //! transfer function for the entry node of the given procedure
  //! should manipulate incoming data-flow set in any special ways
  //! for procedure and return outgoing data-flow set for node
  OA_ptr<DataFlow::DataFlowSet> 
      entryTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in);

  //! transfer function for the exit node of the given procedure
  //! should manipulate outgoing data-flow set in any special ways
  //! for procedure and return incoming data-flow set for node
  OA_ptr<DataFlow::DataFlowSet> 
      exitTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> out);

  //! Propagate a data-flow set from caller to callee
  OA_ptr<DataFlow::DataFlowSet> callerToCallee(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee);
  
  //! Propagate a data-flow set from callee to caller
  OA_ptr<DataFlow::DataFlowSet> calleeToCaller(ProcHandle callee,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle caller);

  //! Propagate a data-flow set from call node to return node
  OA_ptr<DataFlow::DataFlowSet> callToReturn(ProcHandle caller,
                                     OA_ptr<DataFlow::DataFlowSet> dfset, 
                                     CallHandle call, ProcHandle callee);

  //------------------------------------------------------------------
  // Helper routines for the callbacks
  //------------------------------------------------------------------
  
  //! Sets mMemRef2ReachConst[useMemRef] for useMemRefs in given statement
  //! ConstDefSet, in, should not be changed
  void setUseMemRef2Const(ProcHandle, StmtHandle, const ConstDefSet& in);

  //! Sets mMemRef2ReachConst[defMemRef] for defMemRefs in given statement
  //! ConstDefSet, in, should not be changed
  void setDefMemRef2Const(ProcHandle, StmtHandle, const ConstDefSet& in);


private:
  OA_ptr<InterReachConsts> mInterReachConsts;
  OA_ptr<ReachConstsIRInterface> mIR;
  OA_ptr<DataFlow::ParamBindings> mParamBind;
  OA_ptr<Alias::InterAliasInterface> mInterAlias;
  OA_ptr<DataFlow::ICFGDFSolver> mSolver;
  OA_ptr<ICFG::ICFGInterface> mICFG;
  OA_ptr<SideEffect::InterSideEffectInterface> mInterSE;

  std::map<ProcHandle,OA_ptr<ReachConstsStandard> > mReachConstsMap;

};

  } // end of ReachConsts namespace
} // end of OA namespace

#endif
