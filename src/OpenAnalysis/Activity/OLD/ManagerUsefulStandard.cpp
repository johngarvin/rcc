/*! \file
  
  \brief The AnnotationManager that generates UsefulStandard.

  \authors Michelle Strout
  \version $Id: ManagerUsefulStandard.cpp,v 1.10 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerUsefulStandard.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Activity {

static bool debug = false;

/*!
*/
ManagerUsefulStandard::ManagerUsefulStandard(OA_ptr<ActivityIRInterface> _ir) 
    : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerUsefulStandard:ALL", debug);
    mSolver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Backward,*this);
}

OA_ptr<DataFlow::DataFlowSet> ManagerUsefulStandard::initializeTop()
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> >  retval;
    retval = new DataFlow::DataFlowSetImpl<int>;
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> ManagerUsefulStandard::initializeBottom()
{
    // FIXME: really should be all accessible locations, but I don't think
    // this is used, if it is there will be an assertian
    OA_ptr<DataFlow::DataFlowSetImpl<int> >  retval;
    //retval = new DataFlow::LocDFSet;
    return retval;
}

/*!
    Uses the CFGDFProblem::solve functionality to get In and Out
    Useful sets for each basic block.   In transfer function 
    put InUseful locs in UsefulStandard as well.
*/
OA_ptr<UsefulStandard> ManagerUsefulStandard::performAnalysis(ProcHandle proc, 
    OA_ptr<CFG::CFGInterface> cfg, OA_ptr<ICFGDep> dep,
    OA_ptr<DataFlow::DataFlowSetImpl<int> > depTagSet,
    DataFlow::DFPImplement algorithm)
{
  if (debug) {
    std::cout << "In ManagerUsefulStandard::performAnalysis" << std::endl;
    //cfg->dump(std::cout,mIR);
  }
  mUsefulMap = new UsefulStandard(proc);

  // store Dep information for use within the transfer function
  mDep = dep;
  
  // store iterator over independent locations
  //OA_ptr<DataFlow::DataFlowSetImplIterator<int> > tagIter(depTagSet);
  OA_ptr<DataFlow::DataFlowSetImplIterator<int> > tagIter;
  mDepTagIter = tagIter;

  // store CFG for use in initialization
  mCFG = cfg;

  // use the dataflow solver to get the In and Out sets for the BBs
  //DataFlow::CFGDFProblem::solve(cfg);
  mSolver->solve(cfg,algorithm);

  // get the final useful set from the entry node
  OA_ptr<CFG::NodeInterface> entry = cfg->getEntry();
//  OA_ptr<DataFlow::DataFlowSet> dfset = mNodeOutSetMap[entry];
  OA_ptr<DataFlow::DataFlowSet> dfset = mSolver->getOutSet(entry);

  //  No routine is using this FinalUseful set, so depredated in UsefulStandard
  //  All code is still there, but commented out. BK 8/06
  //  mUsefulMap->mapFinalUseful(dfset.convert<DataFlow::LocDFSet>());
  assert(0);
 
  return mUsefulMap;

}

//------------------------------------------------------------------
// Implementing the callbacks for CFGDFProblem
//------------------------------------------------------------------
/*void ManagerUsefulStandard::initializeNode(OA_ptr<CFG::Interface::Node> n)
{
    mNodeInSetMap[n] = new DataFlow::LocDFSet;
    mNodeOutSetMap[n] = new DataFlow::LocDFSet;

    // put independent variables in the In and Out Set for last node
    // and into UsefulStandard that we are creating
    if (n.ptrEqual(mCFG->getExit())) {
        OA_ptr<DataFlow::LocDFSet> temp;
        temp = new DataFlow::LocDFSet;
        
        for (mDepLocIter->reset(); mDepLocIter->isValid(); ++(*mDepLocIter) ) {
          OA_ptr<Location> loc = mDepLocIter->current();
          mUsefulMap->insertDepLoc(loc);
          temp->insert(loc);
       }
       mNodeInSetMap[n] = temp->clone();
       mNodeOutSetMap[n] = temp->clone();
    }
} */


/*!
 *  *  *    Not doing anything special at entries and exits.
 *   *   *     */
OA_ptr<DataFlow::DataFlowSet>
ManagerUsefulStandard::initializeNodeIN(OA_ptr<CFG::NodeInterface> n)
{
     OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
     retval = new DataFlow::DataFlowSetImpl<int>;

     if (n.ptrEqual(mCFG->getExit())) {
        OA_ptr<DataFlow::DataFlowSetImpl<int> > temp;
        temp = new DataFlow::DataFlowSetImpl<int>;

        for (mDepTagIter->reset(); mDepTagIter->isValid(); ++(*mDepTagIter) ) {
            int tag = mDepTagIter->current();
            mUsefulMap->insertDepTag(tag);
            temp->insert(tag);
        }
        retval = temp->clone().convert<DataFlow::DataFlowSetImpl<int> >();
      }
     
     return retval;
}

OA_ptr<DataFlow::DataFlowSet>
ManagerUsefulStandard::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n)
{
      OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
      retval = new DataFlow::DataFlowSetImpl<int>;

      if (n.ptrEqual(mCFG->getExit())) {
         OA_ptr<DataFlow::DataFlowSetImpl<int> > temp;
         temp = new DataFlow::DataFlowSetImpl<int>;

         for (mDepTagIter->reset(); mDepTagIter->isValid(); ++(*mDepTagIter) ) {
              int tag = mDepTagIter->current();
              mUsefulMap->insertDepTag(tag);
              temp->insert(tag);
          }
          retval = temp->clone().convert<DataFlow::DataFlowSetImpl<int> >();
       }
      
      return retval;
}



OA_ptr<DataFlow::DataFlowSet> 
ManagerUsefulStandard::meet (OA_ptr<DataFlow::DataFlowSet> set1orig, 
                             OA_ptr<DataFlow::DataFlowSet> set2orig)
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> > set1
        = set1orig.convert<DataFlow::DataFlowSetImpl<int> >();
    if (debug) {
        std::cout << "ManagerUsefulStandard::meet" << std::endl;
        std::cout << "\tset1 = ";
        set1->dump(std::cout,mIR);
        std::cout << ", set2 = ";
        set2orig->dump(std::cout,mIR);
    }
       
    DataFlow::DataFlowSetImpl<int> retval 
        = set1->unionEqu(*set2orig);
    if (debug) {
        std::cout << std::endl << "\tretval set = ";
        retval.dump(std::cout,mIR);
        std::cout << std::endl;
    }
       
    return retval.clone();
}

/*!
    \brief Core of analysis algorithm.  Transfer function for a stmt.

    KILL is the set of locations that are must defined in this stmt

    GEN is all locations whose use might is needed for locations in 
        OutUseful that may be defined in the statement
*/

OA_ptr<DataFlow::DataFlowSet> 
ManagerUsefulStandard::transfer(OA_ptr<DataFlow::DataFlowSet> out, 
                              OA::StmtHandle stmt) 
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> > outRecast 
        = out.convert<DataFlow::DataFlowSetImpl<int> >();
    OA_ptr<DataFlow::DataFlowSetImpl<int> > inUseful;
    inUseful = new DataFlow::DataFlowSetImpl<int>;

    if (debug) {
        std::cout << "In transfer, stmt(hval=" << stmt.hval() << ")= ";
        mIR->dump(stmt,std::cout);
    }
    
    // loop through all calls in the statement and record the OutVary
    // set for them
    OA_ptr<IRCallsiteIterator> callsiteItPtr = mIR->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
        CallHandle call = callsiteItPtr->current();

        mUsefulMap->copyIntoCallOutUseful(call, outRecast);
    }

    // use dep pairs to determine what locations should be in  InUseful 
    DataFlow::DataFlowSetImplIterator<int> outTagIter(outRecast);
    for ( ; outTagIter.isValid(); ++outTagIter ) {
      int tag = outTagIter.current();
      if (debug) { std::cout << "\toutLoc = ";  
                   std::cout << tag << std::endl; }
      OA_ptr<DataFlow::DataFlowSetImplIterator<int> > tagIterPtr;
      tagIterPtr = mDep->getDiffUseIterator(stmt,tag);
      for ( ; tagIterPtr->isValid(); (*tagIterPtr)++ ) {
          inUseful->insert(tagIterPtr->current());
          /*
          if (debug) { 
            std::cout << "\tinserting into diffUseSet = ";  
            tagIterPtr->current().dump(std::cout,mIR); 
          }
          */
      }
    }

    // map to statement in results
    mUsefulMap->copyIntoInUseful(stmt, inUseful);
   
    return inUseful;
}

  } // end of namespace Activity
} // end of namespace OA
