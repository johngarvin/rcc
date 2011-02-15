/*! \file
  
  \brief Declarations of an AnnotationManager that generates an
         ActivePerStmt by doing Vary analysis and using results
         of Useful analysis.

  \authors Michelle Strout
  \version $Id: ManagerICFGVaryActive.hpp,v 1.1 2005/07/01 02:49:56 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerICFGVaryActive_H
#define ManagerICFGVaryActive_H

//--------------------------------------------------------------------
#include <cassert>

// OpenAnalysis headers
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/IRInterface/ActivityIRInterface.hpp>
#include <OpenAnalysis/DataFlow/ICFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/ICFGDFSolver.hpp>
#include <OpenAnalysis/DataFlow/ParamBindings.hpp>
#include <OpenAnalysis/Activity/InterVary.hpp>
#include <OpenAnalysis/Activity/InterUseful.hpp>
//#include <OpenAnalysis/Activity/ICFGDep.hpp>
#include <OpenAnalysis/Activity/ManagerICFGUseful.hpp>
#include <OpenAnalysis/Activity/ActivePerStmt.hpp>

namespace OA {
  namespace Activity {

/*! 
    Creates ActivePerStmt, which can be queried for the sets of locs
    that are InActive and/or OutActive for each stmt.
*/
class ManagerICFGVaryActive : public virtual DataFlow::ICFGDFProblem
{ 
public:
  ManagerICFGVaryActive(OA_ptr<ActivityIRInterface> _ir);
  ~ManagerICFGVaryActive () {}

  OA_ptr<ActivePerStmt> 
  performAnalysis(OA_ptr<ICFG::ICFGInterface> icfg,
                  OA_ptr<DataFlow::ParamBindings> paramBind,
                  OA_ptr<Alias::Interface> Alias,
                  OA_ptr<InterUseful> interUseful,
                  DataFlow::DFPImplement algorithm);


private:
  //========================================================
  // helper routines
  //========================================================
  OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > calculateActive(
        OA_ptr<DataFlow::DFSetBitImplIterator<Alias::AliasTag> > varyIter,
        OA_ptr<DataFlow::DFSetBitImplIterator<Alias::AliasTag> > usefulIter, StmtHandle stmt);

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


  OA_ptr<DataFlow::DataFlowSet> 
      transfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in,
               OA::CallHandle call);


  
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
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee);

private:
  OA_ptr<ActivePerStmt> mActive;
  OA_ptr<InterUseful> mInterUseful;
  OA_ptr<ActivityIRInterface> mIR;
  //OA_ptr<ICFGDep> mICFGDep;
  OA_ptr<Alias::Interface> mAlias;
  int mMaxNumTags;
  OA_ptr<DataFlow::ICFGDFSolver> mSolver;
  OA_ptr<ICFG::ICFGInterface> mICFG;
  OA_ptr<DataFlow::ParamBindings> mParamBind;

  //! Helper routine for precomputing data-flow sets for each statement.
  void precalcDefsAndUses(OA_ptr<ICFG::ICFGInterface> icfg);
  OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> >  
    calcDefs(OA::OA_ptr<OA::AssignPairIterator> aPairIter);
  OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> >        
    calcMustDefs(OA::OA_ptr<OA::AssignPairIterator> aPairIter);
  OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > 
      calcUses(OA::OA_ptr<OA::AssignPairIterator> aPairIter);

   // Places to keep the precomputing data-flow sets.
  // FIXME: copied from ManagerICFGUseful, ripe for refactoring
  std::map<StmtHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > >
      mStmtToDefs;
  std::map<StmtHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > >
      mStmtToMustDefs;
  std::map<StmtHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > >
      mStmtToUses;

  std::map<CallHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > >
      mCallToDefs;
  std::map<CallHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > >
      mCallToMustDefs;
  std::map<CallHandle,OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > >
      mCallToUses;

};

  } // end of Activity namespace
} // end of OA namespace

#endif
