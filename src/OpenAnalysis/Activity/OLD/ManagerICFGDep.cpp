/*! \file
  
  \brief The AnnotationManager that generates ICFGDep.

  \authors Michelle Strout
  \version $Id: ManagerICFGDep.cpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerICFGDep.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace Activity {

static bool debug = false;


/*!
*/
ManagerICFGDep::ManagerICFGDep(OA_ptr<ActivityIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerICFGDep:ALL", debug);
    mSolver = new DataFlow::ICFGDFSolver(DataFlow::ICFGDFSolver::Forward,*this);
}


/*!
    In transfer function creates a DepDFSet for each stmt.
*/
OA_ptr<ICFGDep> ManagerICFGDep::performAnalysis(
                    OA_ptr<ICFG::ICFGInterface> icfg,
                    OA_ptr<DataFlow::ParamBindings> paramBind,
                    OA::OA_ptr<OA::Alias::Interface> alias,
                    DataFlow::DFPImplement algorithm)
{
  if (debug) {
    std::cout << "In ManagerICFGDep::performAnalysis" << std::endl;
  }
  mDep = new ICFGDep();

  // store results that will be needed in callbacks
  mICFG = icfg;
  mAlias = alias;
  
  // call iterative data-flow solver for ICFG
  mSolver->solve(icfg,algorithm);

  return mDep;

}

//========================================================
// implementation of ICFGDFProblem interface
//========================================================
//--------------------------------------------------------
// initialization callbacks
//--------------------------------------------------------
OA_ptr<DataFlow::DataFlowSet>  
ManagerICFGDep::initializeTop()
{
    OA_ptr<DepDFSet>  retval;
    retval = new DepDFSet;
    return retval;
}

/*!
   Not doing anything special at entries and exits.
 */
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGDep::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n)
{
    return initializeTop();
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGDep::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n)
{
    return initializeTop();
}

//--------------------------------------------------------
// solver callbacks 
//--------------------------------------------------------
 
/*! Not propagating any useful information over the ICFG
*/
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGDep::meet (OA_ptr<DataFlow::DataFlowSet> set1, 
                      OA_ptr<DataFlow::DataFlowSet> set2)
{
    return set1;
}

/*!
    \brief Core of analysis algorithm.  Transfer function for a stmt.

    Create a DepDFSet for this stmt. 
*/

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGDep::transfer(ProcHandle proc,
                         OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt)
{
    // new DepDFSet for this stmt
    OA_ptr<DepDFSet> stmtDepDFSet;
    stmtDepDFSet = new DepDFSet;

    OA_ptr<MemRefExprIterator> usemreIterPtr, defmreIterPtr;

    //! Get the DiffUses in the Statement
    usemreIterPtr = mIR->getDiffUseMREs(stmt);
    //! Get the Defs in the Statement
    defmreIterPtr = mIR->getDefMREs(stmt);

    //! Iterate over DiffUses
    for( ; usemreIterPtr->isValid(); ++(*usemreIterPtr)) {
        OA_ptr<MemRefExpr> usemre = usemreIterPtr->current();

        //! Get the Alias Tags Iterator for each DiffUse
        OA::OA_ptr<OA::Alias::AliasTagSet> diffUsetagSet;
        diffUsetagSet = mAlias->getAliasTags(usemre);
        OA::OA_ptr<OA::Alias::AliasTagIterator> diffUsetagSetIter;
        diffUsetagSetIter = diffUsetagSet->getIterator();


        //! Get the Alias Tags Iterator for each DefTag
        for( defmreIterPtr->reset() ;defmreIterPtr->isValid(); 
             ++(*defmreIterPtr)){

            OA_ptr<MemRefExpr> defmre = defmreIterPtr->current();
            OA::OA_ptr<OA::Alias::AliasTagSet> deftagSet;
            deftagSet = mAlias->getAliasTags(defmre);
            OA::OA_ptr<OA::Alias::AliasTagIterator> deftagSetIter;
            deftagSetIter = deftagSet->getIterator();

            //! Create a map between DiffUse and Def
            for( ; diffUsetagSetIter->isValid(); ++(*diffUsetagSetIter) ) {
               for( ; deftagSetIter->isValid(); ++(*deftagSetIter) ) {
                 stmtDepDFSet->insertDep(diffUsetagSetIter->current(), 
                                         deftagSetIter->current()); 
                 stmtDepDFSet->removeImplicitDep(deftagSetIter->current(),
                                                 deftagSetIter->current());
                   /* PLM 01/03/08
                   if((deftagSetIter->current()).isMust()) {
                      mDep->insertMustDefForStmt(stmt,deftagSetIter->current());
                   }
                   */
                   mDep->insertMustDefForStmt(stmt,deftagSetIter->current());
               }
            } 
        }

    } 

    // map stmtDepDFSet to stmt in depResults
    mDep->mapStmtToDeps(stmt, stmtDepDFSet);

    return in;

}

  } // end of namespace Activity
} // end of namespace OA
