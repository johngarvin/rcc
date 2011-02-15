/*! \file
  
  \brief The AnnotationManager that generates ActivePerStmt.

  \authors Michelle Strout
  \version $Id: ManagerICFGVaryActive.cpp,v 1.1 2005/07/01 02:49:56 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerICFGVaryActive.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Activity {

static bool debug = false;

/*!
*/
ManagerICFGVaryActive::ManagerICFGVaryActive( 
        OA_ptr<ActivityIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerICFGVaryActive:ALL", debug);
    mSolver = new DataFlow::ICFGDFSolver(DataFlow::ICFGDFSolver::Forward,*this);
}

OA_ptr<Activity::ActivePerStmt> 
ManagerICFGVaryActive::performAnalysis(
        OA_ptr<ICFG::ICFGInterface> icfg,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        OA_ptr<Alias::Interface> Alias,
        OA_ptr<ICFGDep> icfgDep,
        OA_ptr<InterUseful> interUseful,
        DataFlow::DFPImplement algorithm)
{
  // store results that will be needed in callbacks
  mICFG = icfg;
  mICFGDep = icfgDep;
  mAlias = Alias;
  mInterUseful = interUseful;

  // create an empty ActivePerStmt
  mActive = new ActivePerStmt();

  // call iterative data-flow solver for ICFG
  mSolver->solve(icfg,algorithm);

  OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
  OA_ptr<DataFlow::DataFlowSetImplIterator<int> > usefulIter;
  OA_ptr<DataFlow::DataFlowSetImpl<int> > varySet;

  // For each ICFG node:
  OA_ptr<ICFG::NodesIteratorInterface> nodeIter =
    icfg->getICFGNodesIterator();
  for ( ; nodeIter->isValid(); (*nodeIter)++) {
    OA_ptr<ICFG::NodeInterface> icfgNode = nodeIter->currentICFGNode();
    // get alias and active results for current procedure
    ProcHandle proc = icfgNode->getProc();
    OA_ptr<UsefulStandard> useful = mInterUseful->getUsefulResults(proc);

    // For each stmt in the ICFG node:
    OA_ptr<CFG::NodeStatementsIteratorInterface> stmtIter =
      icfgNode->getNodeStatementsIterator();

    // for the first statement InActive and OutActive are
    // manual calculations.

    if( stmtIter->isValid() ) {

        StmtHandle stmt = stmtIter->current();

        // calculate InActive
        retval = new DataFlow::DataFlowSetImpl<int>();;
        usefulIter = useful->getInUsefulIterator(stmt);
        varySet = mActive->getInActiveSet(stmt);
        
        DataFlow::DataFlowSetImplIterator<int> varyIter(varySet);
        for ( ; varyIter.isValid(); ++varyIter ) {
             int varyTag = varyIter.current();
             usefulIter->reset();
             for ( ; usefulIter->isValid(); (*usefulIter)++ ) {
                int usefulTag = usefulIter->current();
                if ( varyTag == usefulTag ) {
                    retval->insert(varyTag);
                    retval->insert(usefulTag);
                }
             }
        }
        mActive->copyIntoInActive(stmt, retval);

        // calculate OutActive.
        retval = new DataFlow::DataFlowSetImpl<int>();
        usefulIter = useful->getOutUsefulIterator(stmt);
        varySet = mActive->getOutActiveSet(stmt);
        DataFlow::DataFlowSetImplIterator<int> varyIter1(varySet);
        for ( ; varyIter1.isValid(); ++varyIter1 ) {
           int varyTag = varyIter1.current();
           usefulIter->reset();
           for ( ; usefulIter->isValid(); (*usefulIter)++ ) {
              int usefulTag = usefulIter->current(); 
             if ( varyTag == usefulTag ) {
                 retval->insert(varyTag);
                 retval->insert(usefulTag);
             }
           }
        }
        mActive->copyIntoOutActive(stmt, retval);
        (*stmtIter)++;
    }

   for ( ; stmtIter->isValid(); (*stmtIter)++) {
      StmtHandle stmt = stmtIter->current();

      // OutActive of a predecessor statement is InActive for
      // the successor statement.
      mActive->copyIntoInActive(stmt, retval);

      // Making outActive
      retval = new DataFlow::DataFlowSetImpl<int>();
      usefulIter = useful->getOutUsefulIterator(stmt);
      varySet = mActive->getOutActiveSet(stmt);
      DataFlow::DataFlowSetImplIterator<int> varyIter(varySet);
      for ( ; varyIter.isValid(); ++varyIter ) {
           int varyTag = varyIter.current();
           usefulIter->reset();
           
           for ( ; usefulIter->isValid(); (*usefulIter)++ ) {
             int usefulTag = usefulIter->current();
             if ( varyTag == usefulTag ) {
                 retval->insert(varyTag);
                 retval->insert(usefulTag);
             }
           }
      }
      mActive->copyIntoOutActive(stmt, retval);
    }
  }
  
  mActive->setNumIter(mSolver->getNumIter());
    
  if (debug) {
      std:: cout << std::endl << "%+%+%+%+ Vary numIter = " 
       << mSolver->getNumIter() 
       << " +%+%+%+%" << std::endl << std::endl;
  }
    
  return mActive;
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
OA_ptr<DataFlow::DataFlowSet> ManagerICFGVaryActive::initializeTop()
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    retval = new DataFlow::DataFlowSetImpl<int>();
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n)
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    retval = new DataFlow::DataFlowSetImpl<int>();
    
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n)
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    retval = new DataFlow::DataFlowSetImpl<int>();
    
    return retval;
 }


//--------------------------------------------------------
// solver callbacks 
//--------------------------------------------------------
  
//! OK to modify set1 and return it as result, because solver
//! only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                      OA_ptr<DataFlow::DataFlowSet> set2)
{
    if (debug) {
        std::cout << "ManagerICFGVaryActive::meet" << std::endl;
    }
    OA_ptr<DataFlow::DataFlowSetImpl<int> > remapSet1;
    remapSet1 = set1.convert<DataFlow::DataFlowSetImpl<int> >();
    OA_ptr<DataFlow::DataFlowSetImpl<int> > remapSet2;
    remapSet2 = set2.convert<DataFlow::DataFlowSetImpl<int> >();
    if (debug) {
        std::cout << "\tremapSet1 = ";
        std::cout << remapSet1 << std::endl;
        std::cout << "\tremapSet2 = ";
        std::cout << remapSet2 << std::endl;
    }

    // making sure we don't trounce set1 just in case
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    OA_ptr<DataFlow::DataFlowSet> temp = remapSet1->clone();
    retval = temp.convert<DataFlow::DataFlowSetImpl<int> >();
    *retval = retval->unionEqu(*remapSet2);
    /*
    if (debug) {
        std::cout << "\tretval = ";
        OA_ptr<DataFlow::DataFlowSetImpl<int> > temp;
        temp = retval.convert<DataFlow::DataFlowSetImpl<int> >();
        temp->dump(std::cout, mIR)
    }
    */
    return retval;
}

/*! 
    A helper function that determines active locations, whether
    the previous stmt was active, and which memory references in the
    previous and current stmt are active
*/
OA_ptr<DataFlow::DataFlowSetImpl<int> > 
ManagerICFGVaryActive::calculateActive(
        OA_ptr<DataFlow::DataFlowSetImplIterator<int> > varyIter,
        OA_ptr<DataFlow::DataFlowSetImplIterator<int> > usefulIter, StmtHandle stmt)
{
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    retval = new DataFlow::DataFlowSetImpl<int>();

    if (debug) {
        std::cout << "\tcalculateActive ---------------------" << std::endl;
    }

    // get set of active locations
    varyIter->reset();
    for ( ; varyIter->isValid(); (*varyIter)++ ) {
        int varyTag = varyIter->current();
        if (debug) { 
            std::cout << "\t\tinVary loc = ";
            std::cout << varyTag << std::endl;
        }
        usefulIter->reset();
        for ( ; usefulIter->isValid(); (*usefulIter)++ ) {
            int usefulTag = usefulIter->current();
            if (debug) { 
                std::cout << "\t\tusefulItier loc = ";
                std::cout << usefulTag << std::endl;
            }
            if (varyTag == usefulTag) {
                retval->insert(varyTag);
                retval->insert(usefulTag);
                if (debug) {
                  std::cout << "\t\tinserting active loc = ";
                  std::cout << varyTag << std::endl;
                  std::cout << "\t\tinserting active loc = ";
                  std::cout << usefulTag << std::endl;
                }
            } 
        }
    }

    return retval;
}


//! OK to modify in set and return it again as result because
//! solver clones the BB in sets
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::transfer(ProcHandle proc,
                          OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt)
{    
    // convert incoming set to more specific subclass
    OA_ptr<DataFlow::DataFlowSetImpl<int> > inRecast;
    inRecast = in.convert<DataFlow::DataFlowSetImpl<int> >();
    if (debug) {
        std::cout << "In transfer, stmt(hval=" << stmt.hval() << ")= ";
        mIR->dump(stmt,std::cout);
        std::cout << "\tinRecast = ";
        inRecast->dump(std::cout,mIR);
    }

   // determine the set of incoming active locations based on InUseful
    // and InVary
    mActive->copyIntoInActive(stmt, inRecast);

    // create outVary set
    OA_ptr<DataFlow::DataFlowSetImpl<int> > outVary;
    outVary = new DataFlow::DataFlowSetImpl<int>();

    DataFlow::DataFlowSetImplIterator<int> outTagsIter(inRecast);

    for ( ; outTagsIter.isValid(); ++outTagsIter ) {
      int outTag = outTagsIter.current();
      if (debug) {
          std::cout << "\toutTag = ";
          std::cout << outTag << std::endl;
      }

      OA_ptr<DataFlow::DataFlowSetImplIterator<int> > tagIterPtr;
      tagIterPtr = mICFGDep->getMayDefIterator(stmt,outTag);
      for ( ; tagIterPtr->isValid(); (*tagIterPtr)++ ) {
          outVary->insert(tagIterPtr->current());
      }
    }


    if (debug) {
        std::cout << "\toutVary = ";
        outVary->dump(std::cout,mIR);
    }

  // determine outActive from outVary and outUseful
    mActive->copyIntoOutActive(stmt, outVary);

    return outVary;
}

/*!
   Will get the indep vars if this procedure has any and add them
   to incoming LocDFSet
*/
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::entryTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in)
{
    // create retval as copy of in
    OA_ptr<DataFlow::DataFlowSetImpl<int> > retval;
    OA_ptr<DataFlow::DataFlowSetImpl<int> > inRecast = in.convert<DataFlow::DataFlowSetImpl<int> >();
    retval = new DataFlow::DataFlowSetImpl<int>(*inRecast);

    if (debug) {
      std::cout << "ManagerICFGVaryActive::entryTransfer\n\tinRecast set:";
      inRecast->output(*mIR);
      std::cout << endl;
    }

    // get iterator over indep MemRefExpr for procedure
    OA_ptr<MemRefExprIterator> indepIter = mIR->getIndepMemRefExprIter(proc);
    // get alias results for this procedure

    if (debug) {
      std::cout << "\tindelIter:\n";
    }
    for ( indepIter->reset(); indepIter->isValid(); (*indepIter)++ ) {
      OA_ptr<MemRefExpr> memref = indepIter->current();

      if (debug) {
        std::cout << "\t\tindependent memref: ";
        memref->output(*mIR);
        std::cout << std::endl;
      }
      // get may locs for memref
      OA_ptr<Alias::AliasTagSet> aTag = mAlias->getAliasTags(memref);
      OA_ptr<Alias::AliasTagIterator> tagIter;
      tagIter = aTag->getIterator();
      for ( ; tagIter->isValid(); ++(*tagIter) ) {
        if (debug) {
          std::cout << "\t\t\tinserting AliasTag  : ";
          std::cout << tagIter->current() << std::endl;
          std::cout << "\n";
        }
        retval->insert(tagIter->current());
      }
    }
    if (debug) {
        std::cout << "\tManagerICFGVaryActive, Indep locations for proc "
                  << mIR->toString(proc) << ": " << std::endl;
        retval->output(*mIR);
        std::cout << "----" << std::endl;
    }

    return retval;
}

/*!
   Just pass along out because this won't be called since we are a Forward
   analysis
*/
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::exitTransfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> out)
{
    assert(0);
    return out;
}


//! Propagate a data-flow set from caller to callee
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::callerToCallee(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{
  if (debug) {
    std::cout << "-------- ManagerICFGUseful::callerToCallee" << std::endl;
  }

  return dfset;
}
  
//! Propagate a data-flow set from callee to caller
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::calleeToCaller(ProcHandle callee,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle caller)
{
    return dfset;
}

//! Propagate a data-flow set from call node to return node
OA_ptr<DataFlow::DataFlowSet>
ManagerICFGVaryActive::callToReturn(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee){

    return dfset;
}

  } // end of namespace Activity
} // end of namespace OA
