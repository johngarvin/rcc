/*! \file
  
  \brief The AnnotationManager that generates ReachDefsStandards
         which map each statement to a set of reaching definitions
         that reach that statement.

  \authors Michelle Strout, Andy Stone
  \version $Id: ManagerReachDefsStandard.cpp,v 1.32 2005/06/10 02:32:05 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerReachDefsStandard.hpp"


namespace OA {
  namespace ReachDefs {

static bool debug = false;
using namespace DataFlow;
using namespace Alias;
using namespace std;

ManagerReachDefsStandard::ManagerReachDefsStandard(
    OA_ptr<ReachDefsIRInterface> _ir) 
    : mIR(_ir)
{
   OA_DEBUG_CTRL_MACRO("DEBUG_ManagerReachDefsStandard:ALL", debug);
   mSolver = new CFGDFSolver(CFGDFSolver::Forward,*this);
}


OA_ptr<DataFlowSet> ManagerReachDefsStandard::initializeTop()
{
    OA_ptr<StmtDFSet>  retval;
    retval = new StmtDFSet();
    return retval;
}

OA_ptr<DataFlowSet> ManagerReachDefsStandard::initializeBottom()
{
    OA_ptr<StmtDFSet>  retval;
    retval = new StmtDFSet();
    return retval;
}

/*!
    \brief    Used to perform analysis when not using AQM

    Uses the CFGDFProblem::solve functionality to get In and Out
    reaching definition sets for each basic block.  Then uses the 
    statement transfer function to get an In set for each stmt.
*/
OA_ptr<ReachDefsStandard> ManagerReachDefsStandard::performAnalysis(
    ProcHandle proc, 
    OA_ptr<CFG::CFGInterface> cfg, OA_ptr<Alias::Interface> alias,
    OA_ptr<SideEffect::InterSideEffectInterface> interSE,
    DFPImplement algorithm)
{
  if (debug) {
    std::cout << "In ReachDefs::ManagerReachDefsStandard::performAnalysis"
              << std::endl;
  }
  mReachDefMap = new ReachDefsStandard(proc);

  // store Alias information for use within the transfer function
  mAlias = alias;

  // get mapping of statements to locations they may and must define
  OA_ptr<OA::IRStmtIterator> sIt = mIR->getStmtIterator(proc);
  for ( ; sIt->isValid(); (*sIt)++) {
    OA::StmtHandle stmt = sIt->current();
    if (debug) {
      cout<< "\tstmt (" << stmt.hval() << ") = ";
      mIR->dump(stmt,std::cout);
      cout << endl;
    } 
    
    // initialize each stmt to define an empty set of locations
    mStmtMustDefMap[stmt];
    mStmtMayDefMap[stmt];

    OA_ptr<MemRefExprIterator> defmreIterPtr;
    //! Get the Defs in the Statement
    defmreIterPtr = mIR->getDefMREs(stmt);
    for( defmreIterPtr->reset() ; defmreIterPtr->isValid();
         ++(*defmreIterPtr)){

        OA_ptr<MemRefExpr> defmre = defmreIterPtr->current();

        OA_ptr<Alias::AliasTagSet> tagSet;
        OA_ptr<Alias::AliasTagIterator> tagIterPtr;
        tagSet = alias->getAliasTags(defmre);
        tagIterPtr = tagSet->getIterator();
        for (; tagIterPtr->isValid(); ++(*tagIterPtr)) {
            if(tagSet->isMust()) {
                mStmtMustDefMap[stmt].insert(tagIterPtr->current());
            }

            mStmtMayDefMap[stmt].insert(tagIterPtr->current());
        }
    }
 
    // must or may defines from procedure calls
    OA_ptr<IRCallsiteIterator> callsiteItPtr = mIR->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
      CallHandle expr = callsiteItPtr->current();

      OA_ptr<AliasTagIterator> tagIterPtr;
      
      // MOD
      tagIterPtr = interSE->getMODIterator(expr);
      for ( ; tagIterPtr->isValid(); ++(*tagIterPtr)) {
          mStmtMayDefMap[stmt].insert(tagIterPtr->current());
      }
      
      // DEF
      tagIterPtr = interSE->getDEFIterator(expr);
      for ( ; tagIterPtr->isValid(); ++(*tagIterPtr)) {
          mStmtMustDefMap[stmt].insert(tagIterPtr->current());
      }
    }

  } // loop over statements

  // use the dataflow solver to get the In and Out sets for the BBs
  //DataFlow::CFGDFProblem::solve(cfg);
  //
  mSolver->solve(cfg,algorithm);  
  
  // get exit node for CFG and determine what definitions reach that node
  OA_ptr<CFG::NodeInterface> node;
  
  node = cfg->getExit();

  OA_ptr<DataFlowSet> x = mSolver->getOutSet(node);
      //mNodeInSetMap[node];
  OA_ptr<StmtDFSet> inSet 
    = x.convert<StmtDFSet>();

  OA_ptr<IRHandleIterator<StmtHandle> > rdIter;
  rdIter = new IRHandleIterator<StmtHandle>(inSet);
  
  for (; rdIter->isValid(); ++(*rdIter)) {
      StmtHandle reachDef = rdIter->current();
      mReachDefMap->insertExitReachDef(reachDef);
  }
  return mReachDefMap;
}

//------------------------------------------------------------------
// Implementing the callbacks for CFGDFProblem
//------------------------------------------------------------------
// Not doing anything special at entries and exits.
OA_ptr<DataFlowSet>
ManagerReachDefsStandard::initializeNodeIN(OA_ptr<CFG::NodeInterface> n)
{
     OA_ptr<StmtDFSet>  retval;
     retval = new StmtDFSet();
     return retval;
}

OA_ptr<DataFlowSet>
ManagerReachDefsStandard::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n)
{
     OA_ptr<StmtDFSet>  retval;
     retval = new StmtDFSet();
     return retval;
}



OA_ptr<DataFlowSet> 
ManagerReachDefsStandard::meet (OA_ptr<DataFlowSet> set1orig, 
                       OA_ptr<DataFlowSet> set2orig)
{
    OA_ptr<StmtDFSet> set1, set2, retVal;

    set1 = set1orig.convert<StmtDFSet>();
    set2 = set2orig.convert<StmtDFSet>();

    if (debug) {
        std::cout << "ManagerReachDefsStandard::meet" << std::endl;
        std::cout << "\tset1 = ";
        set1->dump(std::cout,mIR);
        std::cout << endl;
        std::cout << "\tset2 = ";
        set2orig->dump(std::cout,mIR);
        std::cout << endl;
    }


    retVal = new StmtDFSet(*set1);
    retVal->unionEqu(*set2);
    
    if (debug) {
        std::cout << std::endl << "\tretval set = ";
        retVal->dump(std::cout,mIR);
        std::cout << std::endl;
    }
       
    return retVal;
}

/*!
    \brief Core of analysis algorithm.  Transfer function for a stmt.

    KILL is the set of reaching definitions (statements) that may
    define the set of locations that are must defined in this
    statement

    DEF is this statement if it may define anything
*/
OA_ptr<DataFlowSet> 
ManagerReachDefsStandard::transfer(
    OA_ptr<DataFlowSet> in,
    OA::StmtHandle stmt) 
{
    OA_ptr<StmtDFSet> inRecast = in.convert<StmtDFSet>();
    OA_ptr<StmtDFSet> killSet;
    killSet = new StmtDFSet();

    if (debug) {
        cout << "In transfer, stmt(hval=" << stmt.hval() << ")= ";
        mIR->dump(stmt,std::cout);
        cout << endl;
    }
    
    // KILL: will kill all statements that may have defined 
    // locations that this statement must defines
     
    // for each stmt that is a reaching definition
    OA_ptr<IRHandleIterator<StmtHandle> > inIter;
    inIter = new IRHandleIterator<StmtHandle>(inRecast);

    // necessary to iterate this way (w/ while loop) since we
    // can remove elements from the list mid-iteration
    while(inIter->isValid()) {
        StmtHandle reachdef = inIter->current();
        if (debug) {
          cout << "\treachdef in Inset = ";
          mIR->dump(reachdef,std::cout);
          cout << endl;
        }
        
        // insert reachDef into results
        mReachDefMap->insertReachDef(stmt,reachdef);

        // get set of tags that were may defed in reaching
        // definition and are must defed in the current stmt
        OA_ptr<TagsDFSet> killTagSetPtr;
        killTagSetPtr = new TagsDFSet(mStmtMayDefMap[reachdef]);
        killTagSetPtr->intersectEqu(mStmtMustDefMap[stmt]);

        if (debug) {
          cout << "\t\tMayDefs for reachdef stmt(hval=" << stmt.hval() << ")= ";
          //mStmtMayDefMap[reachdef].dump(cout);
          cout << endl;

          cout << "\t\tMustDefs for stmt(hval=" << stmt.hval() << ")= ";
          //mStmtMustDefMap[stmt].dump(cout);
          cout << endl;

          cout << "\t\tkillTagSet for stmt(hval=" << stmt.hval() << ")= ";
          //killTagSetPtr->dump(cout);
          cout << endl;
        }

        // if that set is nonempty and a superset of all the 
        // may defs in the reaching definition then we will go 
        // ahead and kill the reach def
        if (  ! killTagSetPtr->isEmpty() 
             && killTagSetPtr->isSuperset(mStmtMayDefMap[reachdef]))
        {
            // delete statement that is in the kill subset
            killSet->insert(reachdef);
        }
        ++(*inIter);
    }

    // iterate through kill set removing elements from in set
    OA_ptr<IRHandleIterator<StmtHandle> > killIter;
    killIter = new IRHandleIterator<StmtHandle>(killSet);
    for(; killIter->isValid(); ++(*killIter)) {
        StmtHandle reachdef = killIter->current();

        if (debug) {
            std::cout << "\tStmt in Kill set: ";
            mIR->dump(reachdef, std::cout);
            cout << endl;
        }

        inRecast->remove(reachdef);
    }

    // DEF: if this statement has any defs then it is put into the
    //      set of reaching definitions
    
    if (!mStmtMayDefMap[stmt].isEmpty()) {
        // put into dataflow set as a reaching definition
        inRecast->insert(stmt);
        if (debug) {
          cout << "\tStmt in Def set: ";
          mIR->dump(stmt,std::cout);
          cout << endl;
        }
    }

    return inRecast;
}

  } // end of namespace ReachDefs
} // end of namespace OA
