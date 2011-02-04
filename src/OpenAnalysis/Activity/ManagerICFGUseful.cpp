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
        OA_ptr<Alias::Interface> alias,
        DataFlow::DFPImplement algorithm)
{

  // store results that will be needed in callbacks
  mICFG = icfg;
  mAlias = alias;
  mMaxNumTags = mAlias->getMaxAliasTag().val()+1; 
  mParamBind = paramBind;

  // create an empty InterUseful
  mInterUseful = new InterUseful();

  // Precalculate the Defs, Uses, and MustDefs set for each statement
  precalcDefsAndUses(icfg);
  
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
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    return retval;
 }


//--------------------------------------------------------
// solver callbacks 
//--------------------------------------------------------
  
/*! Unions set of useful alias tags.
 */
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                        OA_ptr<DataFlow::DataFlowSet> set2)
{
    if (debug) {
        std::cout << "-------- ManagerICFGUseful::meet" << std::endl;
    }

    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;

    if(set1->size() < set2->size()) {
     retval
      = set1->clone().convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
     retval->unionEqu(*set2);
    } else {
     retval
      = set2->clone().convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
     retval->unionEqu(*set1);
    }

    return retval;
}

//! Given an iterator over a set of assignment pairs return all the
//! may def alias tags.
OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > 
ManagerICFGUseful::calcDefs(OA::OA_ptr<OA::AssignPairIterator> aPairIter)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);

    if (debug) { std::cout << "In ManagerICFGUseful::calcDefs" << std::endl;}
    // ===== Iterate over the AssignPairs ===== 
    for( ; aPairIter->isValid(); ++(*aPairIter)) {
        if (debug) { std::cout << "\tfound assignPair" << std::endl;}

        // ======== Get the LHS (Defs) of Assignment ========
        OA::MemRefHandle memref = aPairIter->currentTarget();

        // ===== Get Defs AliasTagSet ========
        OA_ptr<Alias::AliasTagSet> aSet = mAlias->getAliasTags(memref);
        
        // union it in with retval
        OA_ptr<DataFlow::DataFlowSet> temp = aSet->getDataFlowSet();
        OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > tagset;
        tagset = temp.convert<DataFlow::DataFlowSetImpl<Alias::AliasTag> >();
        DataFlow::DFSetBitImpl<Alias::AliasTag> bitset(mMaxNumTags,tagset->getSet());
        retval->unionEqu(bitset);
    }
    return retval;
}

//! Given an iterator over a set of assignment pairs return all the
//! must def alias tags.
OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > 
ManagerICFGUseful::calcMustDefs(OA::OA_ptr<OA::AssignPairIterator> aPairIter)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    // start with universal because going to intersect in all must defs
    retval->setUniversal();

    // ===== Iterate over the AssignPairs ===== 
    for( ; aPairIter->isValid(); ++(*aPairIter)) {

        // ======== Get the LHS (Defs) of Assignment ========
        OA::MemRefHandle memref = aPairIter->currentTarget();

        // ===== Get Defs AliasTagSet ========
        OA_ptr<Alias::AliasTagSet> aSet = mAlias->getAliasTags(memref);
        
        // intersect it in with retval if these are must defs
        if (aSet->isMust()) {
            // union it in with retval
            OA_ptr<DataFlow::DataFlowSet> temp = aSet->getDataFlowSet();
            OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > tagset;
            tagset = temp.convert<DataFlow::DataFlowSetImpl<Alias::AliasTag> >();
            DataFlow::DFSetBitImpl<Alias::AliasTag> bitset(mMaxNumTags,tagset->getSet());
            retval->intersectEqu(bitset);
        }
    }
    // if there were no must defs, then we need to create an empty set
    if (retval->isUniversalSet()) {
        retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    }
    return retval;
}

//! Given an iterator over a set of assignment pairs return all the
//! use alias tags in the rhs expression.
//! FIXME: what about the uses in the subexpressions for def?
//! FIXME: Also what happened to the concept of differentiable uses?
OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > 
ManagerICFGUseful::calcUses(OA::OA_ptr<OA::AssignPairIterator> aPairIter)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);

    if (debug) { std::cout << "In ManagerICFGUseful::calcUses" << std::endl;}
    // ===== Iterate over the AssignPairs ===== 
    for( ; aPairIter->isValid(); ++(*aPairIter)) {
        if (debug) { std::cout << "\tfound assignPair" << std::endl;}

        //! ======== Get the RHS (Uses) of Assignment ========
        OA::ExprHandle expr = aPairIter->currentSource();
        
        //! ========= Evaluate Expression Tree =========
        OA_ptr<ExprTree> eTree = mIR->getExprTree(expr);

        CollectMREVisitor evalVisitor;
        eTree->acceptVisitor(evalVisitor);

        // ======= Collect all use MREs from ExprTree =========
        OA::set<OA_ptr<MemRefExpr> > mSet;
        mSet = evalVisitor.getMemRef();
        // FIXME: ok this is a little weird
        OA::OA_ptr<std::set<OA_ptr<MemRefExpr> > > mSetPtr;
        mSetPtr = new std::set<OA_ptr<MemRefExpr> >(mSet);

        OA_ptr<MemRefExprIterator> mreIter;
        mreIter = new OA::MemRefExprIterator(mSetPtr);

        // ===== Iterate over the Use MRE =====
        for( ; mreIter->isValid(); ++(*mreIter) ) {
            OA::OA_ptr<OA::MemRefExpr> mre;
            mre = mreIter->current();

            // === ignore AddressOf because it is not a Use ===
            if(mre->isaAddressOf()) { continue; }

            //! ===== Get the AliasTagSet for Use MREs =====
            OA::OA_ptr<OA::Alias::AliasTagSet> mretagSet;
            mretagSet = mAlias->getAliasTags(mreIter->current());

            OA_ptr<DataFlow::DataFlowSet> temp = mretagSet->getDataFlowSet();
            OA_ptr<DataFlow::DataFlowSetImpl<Alias::AliasTag> > tagset;
            tagset = temp.convert<DataFlow::DataFlowSetImpl<Alias::AliasTag> >();
            DataFlow::DFSetBitImpl<Alias::AliasTag> bitset(mMaxNumTags,tagset->getSet());
            retval->unionEqu(bitset); 

        } // end mreIter
 
    }
    return retval;
}

void ManagerICFGUseful::precalcDefsAndUses(OA_ptr<ICFG::ICFGInterface> icfg)
{
  // iterate over ICFG nodes
  OA_ptr<ICFG::NodesIteratorInterface> nodeIter;
  nodeIter = icfg->getICFGNodesIterator();
  for (; nodeIter->isValid(); ++(*nodeIter)) {
    OA_ptr<ICFG::NodeInterface> node = nodeIter->currentICFGNode();
    
    // Do a different initialization based on the ICFG node type
    if(node->getType()==ICFG::CALL_NODE) {
        CallHandle call = node->getCall();
        // ======= Get AssignPairs for the call ========
        OA::OA_ptr<OA::AssignPairIterator> aPairIter;
        aPairIter = mIR->getAssignPairIterator(call);

        mCallToDefs[call] = calcDefs(aPairIter);
        aPairIter = mIR->getAssignPairIterator(call);
        //aPairIter->reset();
        mCallToMustDefs[call] = calcMustDefs(aPairIter);
        aPairIter = mIR->getAssignPairIterator(call);
        //aPairIter->reset();
        mCallToUses[call] = calcUses(aPairIter);

    } else {
        // iterate over the statements in each node
        OA_ptr<CFG::NodeStatementsIteratorInterface> stmtIterPtr 
            = node->getNodeStatementsIterator();
        for (; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
            OA::StmtHandle stmt = stmtIterPtr->current();

            // ======= Get AssignPairs for the Statement ========
            OA::OA_ptr<OA::AssignPairIterator> aPairIter;
            aPairIter = mIR->getAssignPairIterator(stmt);

            mStmtToDefs[stmt] = calcDefs(aPairIter);
            aPairIter = mIR->getAssignPairIterator(stmt);
            mStmtToMustDefs[stmt] = calcMustDefs(aPairIter);
            aPairIter = mIR->getAssignPairIterator(stmt);
            mStmtToUses[stmt] = calcUses(aPairIter);
        }
    }
  }

}

/*! 

  transfer function for useful analysis

  // Kill
  inUseful = outUseful - MustDefs[stmt]

  // Gen
  if (outUseful intersect Defs[stmt])!=empty
    inUseful = inUseful union Uses[stmt]
  else
    inUseful = inUseful

  To handle multiple assignment pairs, have the following definition 
  for MustDefs.
    MustDefs = intersection_over_assign_pairs MustDefs[assign pair]

  Can and do precalculate Defs, Uses, and MustDefs for the statement.
 
  Development notes:
  OK to modify in set and return it again as result because
  solver clones the BB in sets
*/
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::transfer(ProcHandle proc,
                        OA_ptr<DataFlow::DataFlowSet> out, OA::StmtHandle stmt)
{   

    // ===== convert incoming set to more specific subclass =====
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outUseful;
    outUseful = out.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();

    // inUseful = outUseful
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > inUseful;
    inUseful = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*outUseful);

    if (debug) {
        std::cout << "In transfer, stmt (" << mIR->toString(stmt) << ")";
        std::cout << "\n\toutUseful = ";
        outUseful->dump(std::cout,mIR);
        std::cout << std::endl;
    }

    // ===============================================================
    // store outUseful set (below stmt) for stmt into procedure's
    // Useful result
    // ===============================================================
    if (mUsefulMap[proc].ptrEqual(0)) {
      mUsefulMap[proc] = new UsefulStandard(proc);
    }
    mUsefulMap[proc]->copyIntoOutUseful(stmt,outUseful);

    // Kills
    // inUseful = outUseful - MustDefs[stmt]
    // note that inUseful was initialized to outUseful
    inUseful->minusEqu(*(mStmtToMustDefs[stmt]));

    // outUseful intersect Defs[stmt]
    outUseful->intersectEqu(*(mStmtToDefs[stmt]));
    if (debug) { 
        std::cout << "\n\tmStmtToDefs[stmt] = ";
        mStmtToDefs[stmt]->dump(std::cout,mIR);
    }

    if (! outUseful->isEmpty()) {
        if (debug) {
            std::cout << "OutUseful has non-empty intersect with defs" 
                      << std::endl;
        }
        // inUseful = inUseful union Uses[stmt]
        inUseful->unionEqu(*(mStmtToUses[stmt]));
        if (debug) { 
            std::cout << "\n\tmStmtToUses[stmt] = ";
            mStmtToUses[stmt]->dump(std::cout,mIR);
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





OA_ptr<DataFlow::DataFlowSet>
ManagerICFGUseful::transfer(ProcHandle proc, OA_ptr<DataFlow::DataFlowSet> in,
               OA::CallHandle call) 
{

    // ===== convert incoming set to more specific subclass =====
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outUseful;
    outUseful = in.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > inUseful;
    inUseful = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*outUseful);

    if (debug) {
        std::cout << "In transfer, call (" << mIR->toString(call) << ")";
        std::cout << "\n\toutUseful = ";
        outUseful->dump(std::cout,mIR);
    }

    if (mUsefulMap[proc].ptrEqual(0)) {
      mUsefulMap[proc] = new UsefulStandard(proc);
    }
    
    // Kills
    // inUseful = inUseful - MustDefs[call]
    inUseful->minusEqu(*(mCallToMustDefs[call]));

    // outUseful intersect Defs[call]
    outUseful->intersectEqu(*(mCallToDefs[call]));
    if (debug) { 
        std::cout << "\n\tmCallToDefs[call] = ";
        mCallToDefs[call]->dump(std::cout,mIR);
    }

    if (! outUseful->isEmpty()) {
        if (debug) {
            std::cout << "OutUseful has non-empty intersect with defs" 
                      << std::endl;
        }
        // inUseful = outUseful union Uses[call]
        inUseful->unionEqu(*(mCallToUses[call]));
    }

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
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outRecast;
    outRecast = out.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*outRecast);

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
        //mUsefulMap[proc]->insertDepTag(tagIter->current(),mMaxNumTags);
      }
    } // MemRefExpr Iter end


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



//! ===================================================================
//! Propagate a data-flow set from caller to callee
//! It works on 3 parts 
//! 1. Assignment Pair evaluation and transfer
//! 2. Nonlocal transfer
//! 3. Local pass by reference transfer
//! ===================================================================


OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::calleeToCaller(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{
    //! ===== Just pass the entire dataflow set =====

    //! create retval as copy of dfset
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outRecast;
    outRecast = dfset.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*outRecast);
    return retval;
}
  



//! Propagate a data-flow set from callee to caller
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGUseful::callerToCallee(ProcHandle callee,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle caller)
{
    //! ===== Just pass the entire dataflow set =====

    //! create retval as copy of dfset
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outRecast;
    outRecast = dfset.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*outRecast);
    return retval;
}


//! Propagate a data-flow set from call node to return node
OA_ptr<DataFlow::DataFlowSet>
ManagerICFGUseful::callToReturn(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{
    // ===== No need to Pass Anything because calleeToCaller will take care =====

    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    return retval;
}




  } // end of namespace Activity
} // end of namespace OA
