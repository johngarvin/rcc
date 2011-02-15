/*! \file
  
  \brief Declarations of an AnnotationManager that generates an 
         InterReachConsts using the ICFGDFSolver.

  \authors Michelle Strout, Barbara Kreaseck
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
#include <vector>

// OpenAnalysis headers
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/IRInterface/ReachConstsIRInterface.hpp>
#include <OpenAnalysis/DataFlow/ICFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/ICFGDFSolver.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>
#include <OpenAnalysis/DataFlow/ParamBindings.hpp>
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/ExprTree/EvalToConstVisitor.hpp>
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
                  OA_ptr<Alias::Interface> Alias,
                  DataFlow::DFPImplement algorithm);

private:
  
  //! ========================================================
  //! implementation of ICFGDFProblem interface
  //! ========================================================


  //! --------------------------------------------------------
  //! initialization callbacks
  //! --------------------------------------------------------


  //! ===== Return an initialized top set =====
  OA_ptr<DataFlow::DataFlowSet>  initializeTop();
  

  //! ===== Should generate an in and out DataFlowSet for node =====
  OA_ptr<DataFlow::DataFlowSet> 
       initializeNodeIN(OA_ptr<ICFG::NodeInterface> n);

  OA_ptr<DataFlow::DataFlowSet> 
       initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n);
 

  //! --------------------------------------------------------
  //! solver callbacks 
  //! --------------------------------------------------------
 
  //! ===== Ok to modify set1 and set2 ===== 
  OA_ptr<DataFlow::DataFlowSet> meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                                     OA_ptr<DataFlow::DataFlowSet> set2); 


  OA_ptr<DataFlow::DataFlowSet> 
      transfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in, 
               OA::StmtHandle stmt);
 



  OA_ptr<DataFlow::DataFlowSet>
      transfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in,
               OA::CallHandle call);


 
  //! transfer function for the entry node of the given procedure
  //! should manipulate incoming data-flow set in any special ways
  //! for procedure and return outgoing data-flow set for node
  OA_ptr<DataFlow::DataFlowSet> 
      entryTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in);



  //! No needed
  OA_ptr<DataFlow::DataFlowSet> 
      exitTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> out);



  //! ===== Propagate a data-flow set from caller to callee =====
  OA_ptr<DataFlow::DataFlowSet> 
      callerToCallee(ProcHandle caller, OA_ptr<DataFlow::DataFlowSet> dfset, 
                     CallHandle call, ProcHandle callee);
  

  //! ===== Propagate a data-flow set from callee to caller =====
  OA_ptr<DataFlow::DataFlowSet> 
      calleeToCaller(ProcHandle callee, OA_ptr<DataFlow::DataFlowSet> dfset, 
                     CallHandle call, ProcHandle caller);


  //! ===== Propagate a data-flow set from call node to return node =====
  OA_ptr<DataFlow::DataFlowSet> 
      callToReturn(ProcHandle caller, OA_ptr<DataFlow::DataFlowSet> dfset, 
                   CallHandle call, ProcHandle callee);



  //! ===== Needed by transfer =====
  OA_ptr<RCPairSet> transferAssignPair(StmtHandle s, CallHandle c,
                                       bool haveStmt,
                                       MemRefHandle mref, ExprHandle expr,
                                       ProcHandle proc,
                                       OA_ptr<RCPairSet> inSet);



private:
  OA_ptr<InterReachConsts> mInterReachConsts;
  OA_ptr<ReachConstsIRInterface> mIR;
  OA_ptr<Alias::Interface> mAlias;
  OA_ptr<DataFlow::ICFGDFSolver> mSolver;
  OA_ptr<ICFG::ICFGInterface> mICFG;
  std::map<ProcHandle,OA_ptr<ReachConstsStandard> > mReachConstsMap;

};

  } // end of ReachConsts namespace
} // end of OA namespace

#endif
