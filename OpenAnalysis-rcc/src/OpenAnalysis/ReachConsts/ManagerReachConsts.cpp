
/*! \file

  \brief The AnnotationManager that generates ReachConsts,
         which map MemRefHandles and ExprHandles to ConstValBasicInterfaces.

  \authors Michelle Strout, Barbara Kreaseck

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerReachConsts.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace ReachConsts {

#if defined(DEBUG_ALL) || defined(DEBUG_ManagerReachConsts)
static bool debug = true;
static bool extra_debug = true;
static bool meet_debug = true;
static bool transfer_debug = true;
#else
static bool debug = false;
static bool extra_debug = false;
static bool meet_debug = false;
static bool transfer_debug = false;
#endif


ManagerReachConsts::ManagerReachConsts(OA_ptr<ReachConstsIRInterface> _ir)
    : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerReachConstsStandard:ALL", debug);
    mSolver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Forward,*this);
}


OA_ptr<ReachConstsStandard> 
    ManagerReachConsts::performAnalysis(ProcHandle proc,
    OA_ptr<CFG::CFGInterface> cfg, OA_ptr<Alias::Interface> alias,
    DataFlow::DFPImplement algorithm)
{
  mProc = proc;

  // store a new (waiting to be filled) ReachConstsStandard as member
  mRCS = new ReachConstsStandard(proc);

  // store Alias information for use within the transfer function
  mAlias = alias;

  // store CFG for use within initializeNode function
  mCFG = cfg;

  // use the dataflow solver to get
  //    - the In and Out sets for the BBs
  //    - the relationship of memref->reachingConstant (set inside solve)
  //DataFlow::CFGDFProblem::solve(cfg);
  mSolver->solve(cfg,algorithm);

  return mRCS;
}



OA_ptr<DataFlow::DataFlowSet>
ManagerReachConsts::initializeNodeIN(OA_ptr<CFG::NodeInterface> n)
{
    OA_ptr<RCPairSet> retval;
    retval = new RCPairSet();
    return retval;
}

OA_ptr<DataFlow::DataFlowSet>
ManagerReachConsts::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n)
{
    return initializeTop();
}


OA_ptr<DataFlow::DataFlowSet> ManagerReachConsts::initializeTop()
{
    OA_ptr<OA::IRStmtIterator> sIt = mIR->getStmtIterator(mProc);
    for ( ; sIt->isValid(); (*sIt)++) {
      OA::StmtHandle stmt = sIt->current();

      // for each mem ref in this statement
      OA_ptr<MemRefHandleIterator> refIterPtr = mIR->getAllMemRefs(stmt);
      for (; refIterPtr->isValid(); (*refIterPtr)++) {
          MemRefHandle memref = refIterPtr->current();
          OA_ptr<ConstValBasicInterface> nullVal; nullVal = 0;
          OA_ptr<Alias::AliasTagSet> aTag = mAlias->getAliasTags(memref);
          OA_ptr<Alias::AliasTagIterator> tagIter;
          tagIter = aTag->getIterator();
          for ( ; tagIter->isValid(); ++(*tagIter) ) {
             OA_ptr<RCPair> rcPair;
             rcPair = new RCPair(tagIter->current(),nullVal,TOP);
             mAllTop->insert(rcPair); 
          }
      }
    }
    return mAllTop;
}

OA_ptr<DataFlow::DataFlowSet> ManagerReachConsts::initializeBottom()
{
    return mAllTop;
    //return mAllBottom;
}


//! meet routine for ReachConsts
OA_ptr<DataFlow::DataFlowSet>
ManagerReachConsts::meet (OA_ptr<DataFlow::DataFlowSet> set1orig,
                       OA_ptr<DataFlow::DataFlowSet> set2orig)
{
    return set1orig;
}



OA_ptr<DataFlow::DataFlowSet>
ManagerReachConsts::transfer(OA_ptr<DataFlow::DataFlowSet> in, 
                                     OA::StmtHandle stmt)
{
    
  OA_ptr<DataFlow::DataFlowSet> inclone = in->clone();
  OA_ptr<RCPairSet> inRecast = inclone.convert<RCPairSet>();

  //! This loop will add Constant Definitions for every statment
  RCPairSetIterator cdIter(*inRecast);
  for (; cdIter.isValid(); ++(cdIter)) {
         mRCS->insertRCPair(stmt,cdIter.current());
  }

  //! Get AssignPairs 
  //!   Evaluate constant expression
  //!     modify definitions 

  OA_ptr<AssignPairIterator> espIterPtr
         = mIR->getAssignPairIterator(stmt);

  for ( ; espIterPtr->isValid(); (*espIterPtr)++) {

        //! unbundle pair
        MemRefHandle mref = espIterPtr->currentTarget();
        ExprHandle expr = espIterPtr->currentSource();

        OA_ptr<ConstValBasicInterface> cvbiPtr; cvbiPtr = 0;
        OA_ptr<ExprTree> eTreePtr = mIR->getExprTree(expr);

        //! get ConstValBasicInterface by evaluating expression tree
        EvalToConstVisitor evalVisitor(mIR,mRCS);
        eTreePtr->acceptVisitor(evalVisitor);
        cvbiPtr = evalVisitor.getConstVal();

        OA_ptr<Alias::AliasTagSet> aSet = mAlias->getAliasTags(mref);
        OA_ptr<Alias::AliasTagIterator> aIter = aSet->getIterator();
        for ( ; aIter->isValid() ; ++(*aIter)) {
            inRecast->replace(aIter->current(),cvbiPtr,VALUE);
        }
  }

  //return inRecast;
  return inRecast;

}


 } // end of namespace ReachConsts
} // end of namespace OA

