/*! \file
  
  \brief The AnnotationManager that generates InterActive INTERprocedurally.

  \authors Michelle Strout
  \version $Id: ManagerICFGActive.cpp,v 1.3 2005/07/01 02:49:56 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <iostream>
#include "ManagerICFGActive.hpp"
#include <Utils/Util.hpp>



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
   First calls Manager's to get ICFGDep and ICFGUseful results.

   Then does Vary and Activity analysis at same time over the ICFG using
   an ICFGDFSolver.
*/
OA_ptr<InterActive> 
ManagerICFGActive::performAnalysis(
        OA_ptr<ICFG::ICFGInterface> icfg,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        OA_ptr<Alias::Interface> Alias,
        DataFlow::DFPImplement algorithm)
{
  OA_ptr<InterActive> retval;
  retval = new InterActive;

  //! Create a Manager that generates dep information for each 
  //! statement in ICFG
  if (debug) {
      std::cout << "Calling depman->performAnalysis() ...\n";
      std::cout.flush();
  }
  OA_ptr<ManagerICFGDep> depman;
  depman = new ManagerICFGDep(mIR);
  OA_ptr<ICFGDep> icfgDep = depman->performAnalysis(icfg, 
                                                    paramBind,
                                                    Alias,
                                                    algorithm);
  if (debug) { icfgDep->output(*mIR); }


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
                                                               Alias, 
                                                               icfgDep, 
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
                                                          Alias, 
                                                          icfgDep, 
                                                          interUseful,
                                                          algorithm);
  retval->setNumIterVary(active->getNumIter());
  if (debug) { active->output(*mIR); }
  
  
  //-------------------------------------------------------------
  // Do backward dataflow analysis to determine which def memrefs
  // and stmts are active
  
  // store results that will be needed in callbacks
  mAlias = Alias;
  mActive = active;

  // call iterative data-flow solver for ICFG
  mSolver->solve(icfg,algorithm);

  // assign activity results for each procedure into InterActive
  std::map<ProcHandle,OA_ptr<ActiveStandard> >::iterator mapIter;
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
  OA_ptr<OA::DataFlow::DataFlowSetImpl<int> > retval;
  retval = new OA::DataFlow::DataFlowSetImpl<int>; 
  return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n)
{
  OA_ptr<OA::DataFlow::DataFlowSetImpl<int> > retval;
  retval = new OA::DataFlow::DataFlowSetImpl<int>;
  return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGActive::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n)
{
  OA_ptr<OA::DataFlow::DataFlowSetImpl<int> > retval;
  retval = new OA::DataFlow::DataFlowSetImpl<int>;
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

    OA_ptr<DataFlow::DataFlowSetImpl<int> > inActiveSet, outActiveSet, diffSet;

    //! Get the InActive Set for Statement
    inActiveSet = mActive->getInActiveSet(stmt);
    //! Get the OutActive Set for the Statement
    outActiveSet = mActive->getOutActiveSet(stmt);
    diffSet = mActive->getOutActiveSet(stmt);

    OA_ptr<DataFlow::DataFlowSetImplIterator<int> > tagIter;
    tagIter = new DataFlow::DataFlowSetImplIterator<int>(inActiveSet);
    for ( ; tagIter->isValid(); (*tagIter)++ ) {
        active->insertTag(tagIter->current());
    }

    diffSet->differenceEqu(*inActiveSet);
    tagIter = new DataFlow::DataFlowSetImplIterator<int>(diffSet);
    for ( ; tagIter->isValid(); (*tagIter)++ ) {
        active->insertTag(tagIter->current());
    }

    if (debug) {
        std::cout << "In transfer, stmt(hval=" << stmt.hval() << ")= ";
        mIR->dump(stmt,std::cout);
        std::cout << "\tInActive = ";
        inActiveSet->dump(std::cout,mIR);
        std::cout << "\tOutActive = ";
        outActiveSet->dump(std::cout,mIR);
        std::cout << std::endl;
    }

    // iterate over def mem refs and if any of them access a location
    // that is in the out active set then the mem ref is active and so
    // is the stmt
    OA_ptr<MemRefHandleIterator> defIter = mIR->getDefMemRefs(stmt);
    for (; defIter->isValid(); (*defIter)++ ) {
        MemRefHandle def = defIter->current();
        if (debug) {
          std::cout << "\tdef = ";
          mIR->dump(def,std::cout);
        }
        OA_ptr<OA::Alias::AliasTagSet> aliastags;
        aliastags= mAlias->getAliasTags(def);
        OA_ptr<Alias::AliasTagIterator> aTagIter;
        aTagIter = aliastags->getIterator();
        for( ; aTagIter->isValid(); ++(*aTagIter) ) {
            int tag = aTagIter->current();
            if( outActiveSet->contains(tag) ) {
                active->insertMemRef(def);
                // now we know it is, insert and set flag
                active->insertStmt(stmt);
            }
        }
    } 


    // iterate over use mem refs and if any of them access a location
    // that is in the in active set then the mem ref is active 
    OA_ptr<MemRefHandleIterator> useIter = mIR->getUseMemRefs(stmt);
    for (; useIter->isValid(); (*useIter)++ ) {
        MemRefHandle use = useIter->current();
        if (debug) {
          std::cout << "\tuse = ";
          mIR->dump(use,std::cout);
        }
        OA_ptr<OA::Alias::AliasTagSet> aliastags;
        aliastags= mAlias->getAliasTags(use);
        OA_ptr<Alias::AliasTagIterator> aTagIter;
        aTagIter = aliastags->getIterator();
        for( ; aTagIter->isValid(); ++(*aTagIter) ) {
            int tag = aTagIter->current();
            if( inActiveSet->contains(tag) ) {
                active->insertMemRef(use);
            }
        }
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
