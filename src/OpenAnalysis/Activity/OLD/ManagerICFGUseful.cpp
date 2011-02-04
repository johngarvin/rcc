/*! \file
  
  \brief The AnnotationManager that generates InterUseful using ICFGDFSolver.

  \authors Michelle Strout
  \version $Id: ManagerICFGUseful.cpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerICFGUseful.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Activity {

static bool debug = false;

/*!
*/
ManagerICFGUseful::ManagerICFGUseful( OA_ptr<ActivityIRInterface> _ir) : mIR(_ir)
{
  OA_DEBUG_CTRL_MACRO("DEBUG_ManagerICFGUseful:ALL", debug);
  mSolver = new DataFlow::ICFGDFSolver(DataFlow::ICFGDFSolver::Backward,*this);
}

OA_ptr<Activity::InterUseful> 
ManagerICFGUseful::performAnalysis(
        OA_ptr<ICFG::ICFGInterface> icfg,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        OA_ptr<Alias::Interface> Alias,
        OA_ptr<ICFGDep> icfgDep,
        DataFlow::DFPImplement algorithm)
{
  // store results that will be needed in callbacks
  mICFG = icfg;
  mICFGDep = icfgDep;
  mAlias = Alias;
  //! no longer used
  //mInterSE = interSE;

  // create an empty InterUseful
  mInterUseful = new InterUseful();

  // call iterative data-flow solver for ICFG
  mSolver->solve(icfg,algorithm);

  if (debug) {
      std::cout << std::endl << "%+%+%+%+ Useful numIter = " 
                << mSolver->getNumIter() 
                << " +%+%+%+%" << std::endl << std::endl;
  }

  mInterUseful->setNumIter(mSolver->getNumIter());    

  // assign results for each procedure into InterUseful
  std::map<ProcHandle,OA_ptr<UsefulStandard> >::iterator mapIter;
  for (mapIter=mUsefulMap.begin(); mapIter!=mUsefulMap.end(); mapIter++) {
    mInterUseful->mapProcToUseful(mapIter->first,mapIter->second);
  }

  return mInterUseful;
}

//========================================================
// implementation of ICFGDFProblem interface
//========================================================
//--------------------------------------------------------
// initialization callbacks
//--------------------------------------------------------

/*!
   Data-flow set passed around on the call graph is an
   LocDFSet.  The top value for this is an empty set.
*/
OA_ptr<DataFlow::DataFlowSet> ManagerICFGUseful::initializeTop()
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    retval = new DataFlow::DataFlowSetImpl<int>();
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n)
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    retval = new DataFlow::DataFlowSetImpl<int>();
    
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n)
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    retval = new DataFlow::DataFlowSetImpl<int>();
    return retval;
 }


//--------------------------------------------------------
// solver callbacks 
//--------------------------------------------------------
  
/*! Unions location sets
 */
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                        OA_ptr<DataFlow::DataFlowSet> set2)
{
    if (debug) {
        std::cout << "-------- ManagerICFGUseful::meet" << std::endl;
    }
    OA_ptr<DataFlow::DataFlowSetImpl<int> > remapSet1;
    remapSet1 = set1.convert<DataFlow::DataFlowSetImpl<int> >();
    OA_ptr<DataFlow::DataFlowSetImpl<int> > remapSet2;
    remapSet2 = set2.convert<DataFlow::DataFlowSetImpl<int> >();
    if (debug) {
        std::cout << "\tremapSet1 = ";
        remapSet1->dump(std::cout, mIR);
        std::cout << "\tremapSet2 = ";
        remapSet2->dump(std::cout, mIR);
    }

    // making sure we don't trounce set1 just in case
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    OA_ptr<DataFlow::DataFlowSet> temp = remapSet1->clone();
    retval = temp.convert<DataFlow::DataFlowSetImpl<int> >();
    *retval = retval->unionEqu(*remapSet2);
    if (debug) {
        std::cout << "\tretval = ";
        OA_ptr<DataFlow::DataFlowSetImpl<int> > temp;
        temp = retval.convert<DataFlow::DataFlowSetImpl<int> >();
        temp->dump(std::cout, mIR);
    }
    return retval;
}

//! OK to modify in set and return it again as result because
//! solver clones the BB in sets
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::transfer(ProcHandle proc,
                        OA_ptr<DataFlow::DataFlowSet> out, OA::StmtHandle stmt)
{   
    // convert incoming set to more specific subclass
    OA_ptr<DataFlow::DataFlowSetImpl<int> > outUseful;
    outUseful = out.convert<DataFlow::DataFlowSetImpl<int> >();
    OA_ptr<DataFlow::DataFlowSetImpl<int> > inUseful;
    inUseful = new DataFlow::DataFlowSetImpl<int>();
    if (debug) {
        std::cout << "In transfer, stmt (" << mIR->toString(stmt) << ")";
        std::cout << "\n\toutRecast = ";
        outUseful->dump(std::cout,mIR);
    }

    // store outUseful set (below stmt) for stmt into procedure's Useful result
    if (mUsefulMap[proc].ptrEqual(0)) {
      mUsefulMap[proc] = new UsefulStandard(proc);
    }
    mUsefulMap[proc]->copyIntoOutUseful(stmt,outUseful);
    
    // use dep pairs to determine what locations should be in InUseful (above)
    DataFlow::DataFlowSetImplIterator<int> outTagsIter( outUseful );

    for ( ; outTagsIter.isValid(); ++outTagsIter ) {
      int outTag = outTagsIter.current();
      if (debug) {
          std::cout << "\toutLoc = ";
          std::cout << outTag << std::endl;
      }

      OA_ptr<DataFlow::DataFlowSetImplIterator<int> > tagIterPtr;
      tagIterPtr = mICFGDep->getDiffUseIterator(stmt,outTag);
      for ( ; tagIterPtr->isValid(); (*tagIterPtr)++ ) {
          inUseful->insert(tagIterPtr->current());
      } 
    }
    
    // store inUseful (above stmt) for stmt into procedure's Useful results
    mUsefulMap[proc]->copyIntoInUseful(stmt,inUseful);

    if (debug) {
        std::cout << "\tinUseful = ";
        inUseful->dump(std::cout,mIR);
    }
    return inUseful;
}

/*!
   Will get the dep vars if this procedure has any and add them
   to incoming LocDFSet
*/
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::exitTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> out)
{
  if (debug) {
    std::cout << "-------- exitTransfer\n";
  }
    // create retval as copy of out
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    OA_ptr<DataFlow::DataFlowSetImpl<int> > outRecast;
    outRecast = out.convert<DataFlow::DataFlowSetImpl<int> >();
    retval = new DataFlow::DataFlowSetImpl<int>(*outRecast);

   // get iterator over dep MemRefExpr for procedure
    OA_ptr<MemRefExprIterator> depIter = mIR->getDepMemRefExprIter(proc);

    if (mUsefulMap[proc].ptrEqual(0)) {
        mUsefulMap[proc] = new UsefulStandard(proc);
    }
    for ( depIter->reset(); depIter->isValid(); (*depIter)++ ) {
      OA_ptr<MemRefExpr> memref = depIter->current();

      if (debug) {
        std::cout << "    dep memref dump = '";
        memref->dump(std::cout, mIR);
        std::cout << "'\n";
        std::cout << "    dep memref output = '";
        memref->output(*mIR);
        std::cout << "'\n";
      }

      // get may locs for memref
      OA_ptr<Alias::AliasTagSet> aTag = mAlias->getAliasTags(memref);
      OA_ptr<Alias::AliasTagIterator> tagIter;
      tagIter = aTag->getIterator();
      for ( ; tagIter->isValid(); ++(*tagIter)) {
        if (debug) {
          std::cout << "        found dep location dump: '";
          std::cout << tagIter->current() << std::endl;
          std::cout << "'\n";
          std::cout << "        found dep location output: '";
          std::cout << tagIter->current() << std::endl;
          std::cout << "'\n";
        }
        retval->insert(tagIter->current());
        mUsefulMap[proc]->insertDepTag(tagIter->current());
      }
    }
    if (debug) {
        std::cout << "\tManagerICFGUseful, Dep locations for proc "
                  << mIR->toString(proc) << ": " << std::endl;
        retval->dump(std::cout, mIR);
        std::cout << "----" << std::endl;
    }

    return retval;
}

/*!
   Just pass along out because this won't be called since we are a Backward
   analysis
*/
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::entryTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in)
{
    assert(0);
    return in;
}


//! Propagate a data-flow set from caller to callee
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::callerToCallee(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{
    return dfset; 
}
  
//! Propagate a data-flow set from callee to caller
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::calleeToCaller(ProcHandle callee,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle caller)
{
    return dfset;
}

//! Propagate a data-flow set from call node to return node
OA_ptr<DataFlow::DataFlowSet>
ManagerICFGUseful::callToReturn(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{
    return dfset;
}

  } // end of namespace Activity
} // end of namespace OA
