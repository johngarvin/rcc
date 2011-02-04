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


    //! ===========================================
    //! Iterate over assignment pairs of statement
    //! ===========================================

    OA::OA_ptr<OA::AssignPairIterator> aPairIter;
    aPairIter = mIR->getAssignPairIterator(stmt);

    for( ; aPairIter->isValid(); ++(*aPairIter)) {

        //! ======== Get the LHS (Defs) of Assignment ========
        OA::MemRefHandle memref = aPairIter->currentTarget();

        //! ======== Get the RHS (Uses) of Assignment ========
        OA::ExprHandle expr = aPairIter->currentSource();


        //! ========
        //! Get Defs
        //! ========
        OA_ptr<Alias::AliasTagSet> aSet = mAlias->getAliasTags(memref);
        OA::OA_ptr<OA::Alias::AliasTagIterator> deftagSetIter;
        deftagSetIter = aSet->getIterator();



        //! ================ Def tagIter =============
        for( deftagSetIter->reset() ; deftagSetIter->isValid();
             ++(*deftagSetIter) ) {

                 //! =====================================================
                 //! Store Must Defs
                 //!
                 //! Do it here, before going over Uses in RHS ExprHandle.
                 //! because we have pointer assignment statements p=>x
                 //! which is modelled as p=&x.  AddressOf is not a Use
                 //! and therefore we ignore &x for ICFGDep pairs but
                 //! def "p" is Must Def and we dont want to ignore it.
                 //! Thus, process MustDefs before accessing RHS.
                 //! =====================================================

                  if(aSet->isMust()) {
                     mDep->insertMustDefForStmt(stmt,deftagSetIter->current());
                     stmtDepDFSet->removeImplicitDep(deftagSetIter->current(),
                                                     deftagSetIter->current());
                  }

                 //! ============
                 //! Get Uses
                 //! ============

                 //! ======== Get ExprTree for RHS =========
                 OA_ptr<ExprTree> eTree = mIR->getExprTree(expr);

                 CollectMREVisitor evalVisitor;
                 eTree->acceptVisitor(evalVisitor);

                 //! ======= Collect all MREs from ExprTree =========
                 OA::set<OA_ptr<MemRefExpr> > mSet;
                 mSet = evalVisitor.getMemRef();

                 OA::OA_ptr<std::set<OA_ptr<MemRefExpr> > > mSetPtr;
                 mSetPtr = new std::set<OA_ptr<MemRefExpr> >(mSet);

                 OA_ptr<MemRefExprIterator> mreIter;
                 mreIter = new OA::MemRefExprIterator(mSetPtr);



                 //! ============ Use MREIter ==============
                 for( ; mreIter->isValid(); ++(*mreIter) ) {

                     OA::OA_ptr<OA::MemRefExpr> mre;
                     mre = mreIter->current();

                     //! === ignore AddressOf because it is not a Use ===
                     if(mre->isaAddressOf()) {
                        continue;
                     }


                     OA::OA_ptr<OA::Alias::AliasTagSet> mretagSet;
                     OA::OA_ptr<OA::Alias::AliasTagIterator> mretagSetIter;
                     mretagSet = mAlias->getAliasTags(mreIter->current());
                     mretagSetIter = mretagSet->getIterator();

                     //! =============== Use tagIter ==============
                     for( ; mretagSetIter->isValid(); ++(*mretagSetIter)) {

                         //! =============================================
                         //! Create ICFGDep Pair
                         //! =============================================

                         stmtDepDFSet->insertDep(mretagSetIter->current(),
                                                 deftagSetIter->current());

                     } // end mreTagSetIter

                 } // end mreIter

             } // end deftagIter

         } // end aPairIter


         //! =========================================
         //! map stmtDepDFSet to stmt in depResults
         //! =========================================
         mDep->mapStmtToDeps(stmt, stmtDepDFSet);


         return in;

}



//! Propagate a data-flow set from caller to callee
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGDep::callerToCallee(ProcHandle caller,
   OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{

    
    // new DepDFSet for this stmt
    OA_ptr<DepDFSet> stmtDepDFSet;
    stmtDepDFSet = new DepDFSet;


    //! ===========================================
    //! Iterate over assignment pairs of statement
    //! ===========================================

    OA::OA_ptr<OA::AssignPairIterator> aPairIter;
    aPairIter = mIR->getAssignPairIterator(call);

    for( ; aPairIter->isValid(); ++(*aPairIter)) {

        //! ======== Get the LHS (Defs) of Assignment ========
        OA::MemRefHandle memref = aPairIter->currentTarget();

        //! ======== Get the RHS (Uses) of Assignment ========
        OA::ExprHandle expr = aPairIter->currentSource();


        //! ========
        //! Get Defs
        //! ========
        OA_ptr<Alias::AliasTagSet> aSet = mAlias->getAliasTags(memref);
        OA::OA_ptr<OA::Alias::AliasTagIterator> deftagSetIter;
        deftagSetIter = aSet->getIterator();



        //! ================ Def tagIter =============
        for( deftagSetIter->reset() ; deftagSetIter->isValid();
             ++(*deftagSetIter) ) {


                 //! ============
                 //! Get Uses
                 //! ============

                 //! ======== Get ExprTree for RHS =========
                 OA_ptr<ExprTree> eTree = mIR->getExprTree(expr);

                 CollectMREVisitor evalVisitor;
                 eTree->acceptVisitor(evalVisitor);

                 //! ======= Collect all MREs from ExprTree =========
                 OA::set<OA_ptr<MemRefExpr> > mSet;
                 mSet = evalVisitor.getMemRef();

                 OA::OA_ptr<std::set<OA_ptr<MemRefExpr> > > mSetPtr;
                 mSetPtr = new std::set<OA_ptr<MemRefExpr> >(mSet);

                 OA_ptr<MemRefExprIterator> mreIter;
                 mreIter = new OA::MemRefExprIterator(mSetPtr);



                 //! ============ Use MREIter ==============
                 for( ; mreIter->isValid(); ++(*mreIter) ) {

                     OA::OA_ptr<OA::MemRefExpr> mre;
                     mre = mreIter->current();

                     //! === ignore AddressOf because it is not a Use ===
                     if(mre->isaAddressOf()) {
                        continue;
                     }

                     OA::OA_ptr<OA::Alias::AliasTagSet> mretagSet;
                     OA::OA_ptr<OA::Alias::AliasTagIterator> mretagSetIter;
                     mretagSet = mAlias->getAliasTags(mreIter->current());
                     mretagSetIter = mretagSet->getIterator();

                     //! =============== Use tagIter ==============
                     for( ; mretagSetIter->isValid(); ++(*mretagSetIter)) {

                         //! =============================================
                         //! Create ICFGDep Pair
                         //! =============================================

                         stmtDepDFSet->insertDep(mretagSetIter->current(),
                                                 deftagSetIter->current());



                     } // end mreTagSetIter

                 } // end mreIter

        } // end deftagIter


    } // end aPairIter


    //! =========================================
    //! map stmtDepDFSet to stmt in depResults
    //! =========================================
    mDep->mapCallToDeps(call, stmtDepDFSet);


    return dfset;

}




  } // end of namespace Activity
} // end of namespace OA



