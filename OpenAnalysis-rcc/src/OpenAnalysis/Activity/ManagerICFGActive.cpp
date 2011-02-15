/*! \file
  
  \brief The AnnotationManager that generates InterActive INTERprocedurally.

  \authors Michelle Strout

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <iostream>
#include <OpenAnalysis/Activity/ManagerICFGActive.hpp>
#include <Utils/Util.hpp>

#include <sys/time.h>

namespace OA {
  namespace Activity {

static bool debug = false;

ManagerICFGActive::ManagerICFGActive(
    OA_ptr<Activity::ActivityIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerICFGActive:ALL", debug);
    mSolver = new DataFlow::ICFGDFSolver(DataFlow::ICFGDFSolver::Backward,*this);
}

/*!
*/
OA_ptr<InterActiveFortran> 
ManagerICFGActive::performAnalysis(
        OA_ptr<ICFG::ICFGInterface> icfg,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        OA_ptr<Alias::Interface> alias,
        DataFlow::DFPImplement algorithm)
{
  OA_ptr<InterActiveFortran> retval;
  retval = new InterActiveFortran();

  //! ManagerICFGUseful does useful analysis and determines which
  //! AliasTags are useful coming Into a stmt and Outof a stmt
  if (debug) {
      std::cout << "Calling usefulman->performAnalysis() ...\n";
      std::cout.flush();
  }

  OA_ptr<ManagerICFGUseful> usefulman;
  usefulman = new ManagerICFGUseful(mIR);
  OA_ptr<InterUseful> interUseful = usefulman->performAnalysis(icfg, 
                                                               paramBind, 
                                                               alias, 
                                                               algorithm);
  retval->setNumIterUseful(interUseful->getNumIter());
  if (debug) { interUseful->output(*mIR); }
 
  
  // ManagerICFGVaryActive does vary analysis and determines which
  // locations are active coming Into a stmt and Outof a stmt
  if (debug) {
      std::cout << "Calling varyman->performAnalysis() ...\n";
      std::cout.flush();
  }

  OA_ptr<ManagerICFGVaryActive> varyman;
  varyman = new ManagerICFGVaryActive(mIR);
  OA_ptr<ActivePerStmt> active = varyman->performAnalysis(icfg, 
                                                          paramBind,
                                                          alias, 
                                                          interUseful,
                                                          algorithm);

  retval->setNumIterVary(active->getNumIter());
  if (debug) { active->output(*mIR); }
  
  
  //-------------------------------------------------------------
  // Do backward dataflow analysis to determine which def memrefs
  // and stmts are active
  
  // store results that will be needed in callbacks
  mAlias = alias;
  mMaxTags = alias->getMaxAliasTag().val();
  mActive = active;

  // call iterative data-flow solver for ICFG
  mSolver->solve(icfg,algorithm);

  // iterate over all active alias tags and generate the set of active
  // SymHandles
  // for each procedure
  std::map<ProcHandle,OA_ptr<ActiveStandard> >::iterator mapIter;
  for (mapIter=mActiveMap.begin(); mapIter!=mActiveMap.end(); mapIter++) {
    ProcHandle proc = mapIter->first;

    // get alias and active results for current procedure
    OA_ptr<ActiveStandard> active = mActiveMap[proc];

    OA_ptr<DataFlow::DFSetBitImplIterator<Alias::AliasTag> > tagIter;
    tagIter = new DataFlow::DFSetBitImplIterator<Alias::AliasTag>(mActiveSet[proc]);
    for ( ; tagIter->isValid(); (*tagIter)++ ) {
        active->insertTag(tagIter->current());
        OA_ptr<MemRefExprIterator> mreIter
            = mAlias->getMemRefExprIterator(tagIter->current());
     
        for( ; mreIter->isValid(); ++(*mreIter) ) {
            OA_ptr<MemRefExpr> mre 
                = mreIter->current();
            if(mre->isaNamed()) {
               active->insertSym(mre.convert<NamedRef>()->getSymHandle());
            }
            // ASSUMPTION: doing this for FORTRAN reference params
            // Also subseting I guess.
            if(mre->isaRefOp()) {
               while(!mre->isaNamed()) {
                   mre = mre.convert<RefOp>()->getMemRefExpr();
               }
               active->insertSym(mre.convert<NamedRef>()->getSymHandle());
            }
        }
    }

  }

  // assign activity results for each procedure into InterActive
  for (mapIter=mActiveMap.begin(); mapIter!=mActiveMap.end(); mapIter++) {
    retval->mapProcToActive(mapIter->first,mapIter->second);
  }

  // how many iterations did this take?
  if (debug) 
    {
      std::cout << "ICFGActive took " << mSolver->getNumIter()
                << " iterations.\n";
    }

  retval->setNumIterActive(mSolver->getNumIter());

  return retval;
}

//========================================================
// implementation of ICFGDFProblem interface
//========================================================
//--------------------------------------------------------
// initialization callbacks
//--------------------------------------------------------

/*!
   Data-flow set passed around on the icfg is a
   LocDFSet.  The top value for this is an empty set.
*/
OA_ptr<DataFlow::DataFlowSet> ManagerICFGActive::initializeTop()
{
  OA_ptr<OA::DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
  retval = new OA::DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxTags); 
  return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n)
{
  OA_ptr<OA::DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
  retval = new OA::DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxTags);
  return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n)
{
  OA_ptr<OA::DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
  retval = new OA::DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxTags);
  return retval;
}


//--------------------------------------------------------
// solver callbacks 
//--------------------------------------------------------
  
//! OK to modify set1 and return it as result, because solver
//! only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                      OA_ptr<DataFlow::DataFlowSet> set2)
{
    // just return in set
    return set1;
}

//! OK to modify in set and return it again as result because
//! solver clones the BB in sets
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::transfer(ProcHandle proc,
                        OA_ptr<DataFlow::DataFlowSet> out, OA::StmtHandle stmt)
{  
    // will be storing activity results for stmt and memrefs in
    // ActiveStandard for current procedure
    if (mActiveMap[proc].ptrEqual(0)) {
        mActiveMap[proc] = new ActiveStandard(proc);
    }
    // get alias and active results for current procedure
    OA_ptr<ActiveStandard> active = mActiveMap[proc];

    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > inActiveSet, outActiveSet;

    // Get the InActive Set for Statement
    inActiveSet = mActive->getInActiveSet(stmt);
    // Get the OutActive Set for the Statement
    outActiveSet = mActive->getOutActiveSet(stmt);

    // insert both of the above sets into the active alias tags for
    // this procedure
    if (mActiveSet[proc].ptrEqual(0)) {
      mActiveSet[proc] = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*inActiveSet);
      mActiveSet[proc]->unionEqu(*outActiveSet);
    } else {
      mActiveSet[proc]->unionEqu(*inActiveSet);
      mActiveSet[proc]->unionEqu(*outActiveSet);
    }

    // FIXME: MMS, 8/4/08, for now not going to have any active statements
    //if((!inActiveSet->isEmpty()) && (!diffSet->isEmpty())) {
    //    active->insertStmt(stmt);
   // }

/* moved to performAnalysis
   OA_ptr<DataFlow::DFSetBitImplIterator<Alias::AliasTag> > tagIter;
    tagIter = new DataFlow::DFSetBitImplIterator<Alias::AliasTag>(inActiveSet);
    for ( ; tagIter->isValid(); (*tagIter)++ ) {
        active->insertTag(tagIter->current());
        OA_ptr<MemRefExprIterator> mreIter
            = mAlias->getMemRefExprIterator(tagIter->current());
     
        for( ; mreIter->isValid(); ++(*mreIter) ) {
            OA_ptr<MemRefExpr> mre 
                = mreIter->current();
            if(mre->isaNamed()) {
               active->insertSym(mre.convert<NamedRef>()->getSymHandle());
            }
            if(mre->isaRefOp()) {
               while(!mre->isaNamed()) {
                   mre = mre.convert<RefOp>()->getMemRefExpr();
               }
               active->insertSym(mre.convert<NamedRef>()->getSymHandle());
            }
        }
    }

    diffSet->minusEqu(*inActiveSet);
    tagIter = new DataFlow::DFSetBitImplIterator<Alias::AliasTag>(diffSet);
    for ( ; tagIter->isValid(); (*tagIter)++ ) {
        active->insertTag(tagIter->current());
        OA_ptr<MemRefExprIterator> mreIter
            = mAlias->getMemRefExprIterator(tagIter->current());
        for( ; mreIter->isValid(); ++(*mreIter) )
        {
           OA_ptr<MemRefExpr> mre
               = mreIter->current();
           if(mre->isaNamed()) {
              active->insertSym(mre.convert<NamedRef>()->getSymHandle());
           }
           if(mre->isaRefOp()) {
              while(!mre->isaNamed()) {
                   mre = mre.convert<RefOp>()->getMemRefExpr(); 
              }
              active->insertSym(mre.convert<NamedRef>()->getSymHandle());
           }
        }
    }
*/
    if (debug) {
        std::cout << "In transfer, stmt(hval=" << stmt.hval() << ")= ";
        mIR->dump(stmt,std::cout);
        std::cout << "\tInActive = ";
        inActiveSet->dump(std::cout,mIR);
        std::cout << "\tOutActive = ";
        outActiveSet->dump(std::cout,mIR);
        std::cout << std::endl;
    }

    return out;
}

/*!
   Will get the indep vars if this procedure has any and add them
   to incoming LocDFSet
*/
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::entryTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in)
{
    return in;
}

/*!
*/
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::exitTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> out)
{
    return out;
}


//! Propagate a data-flow set from caller to callee
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::callerToCallee(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{
  // no data flow for this analysis ... returning in set
  return dfset;
}
  
//! Propagate a data-flow set from callee to caller
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::calleeToCaller(ProcHandle callee,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle caller)
{
  // no data flow for this analysis ... returning in set
  return dfset;
}

//! Propagate a data-flow set from call node to return node
OA_ptr<DataFlow::DataFlowSet>
ManagerICFGActive::callToReturn(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{
  // no data flow for this analysis ... returning in set
  return dfset;
}

  } // end of namespace Activity
} // end of namespace OA
