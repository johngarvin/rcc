/*! \file
  
  \brief The AnnotationManager that generates ActivePerStmt.

  \authors Michelle Strout, Barbara Kreaseck, Priya Malusare

  First the Vary in and out sets are computed for each statement
  through use of the ICFGDFSolver.
  Then the in and out active sets are computed as one pass
  in the performAnalysis method.

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
        OA_ptr<Alias::Interface> alias,
        OA_ptr<InterUseful> interUseful,
        DataFlow::DFPImplement algorithm)
{
  // store results that will be needed in callbacks
  mICFG = icfg;
  mAlias = alias;
  mMaxNumTags = mAlias->getMaxAliasTag().val()+1;
  mInterUseful = interUseful;
  
  mParamBind = paramBind;

  // create an empty ActivePerStmt
  mActive = new ActivePerStmt();
  
  // Precalculate the Defs, Uses, and MustDefs set for each statement
  // FIXME: doing the same thing in ManagerICFGUseful, can we refactor
  // this?
  precalcDefsAndUses(icfg);

  // call iterative data-flow solver for ICFG
  mSolver->solve(icfg,algorithm);


  OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
  OA_ptr<DataFlow::DFSetBitImplIterator<Alias::AliasTag> > usefulIter;
  OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > varySet;

  // For each ICFG node:
  OA_ptr<ICFG::NodesIteratorInterface> nodeIter =
    icfg->getICFGNodesIterator();
  for ( ; nodeIter->isValid(); (*nodeIter)++) {
    OA_ptr<ICFG::NodeInterface> icfgNode = nodeIter->currentICFGNode();
    // get alias and active results for current procedure
    ProcHandle proc = icfgNode->getProc();
    OA_ptr<UsefulStandard> useful = mInterUseful->getUsefulResults(proc);

    if(icfgNode->getCall() != OA::CallHandle(0)) {

       OA::CallHandle call = icfgNode->getCall();

       // calculate InActive
        retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
        usefulIter = useful->getInUsefulIterator(call);
        varySet = mActive->getInActiveSet(call);

        for ( ; usefulIter->isValid(); (*usefulIter)++ ) {
             Alias::AliasTag usefulTag = usefulIter->current();
             if(varySet->contains(usefulTag)) {
                retval->insert(usefulTag);
             }
        }
        mActive->copyIntoInActive(call, retval);


        // calculate InActive
        retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
        usefulIter = useful->getOutUsefulIterator(call);
        varySet = mActive->getOutActiveSet(call);

        for ( ; usefulIter->isValid(); (*usefulIter)++ ) {
             Alias::AliasTag usefulTag = usefulIter->current();
             if(varySet->contains(usefulTag)) {
                retval->insert(usefulTag);
             }
        }
        mActive->copyIntoOutActive(call, retval);

    } else {

        // For each stmt in the ICFG node:
        OA_ptr<CFG::NodeStatementsIteratorInterface> stmtIter =
          icfgNode->getNodeStatementsIterator();

        // for the first statement InActive and OutActive are
        // manual calculations.

        for( ; stmtIter->isValid() ; ++(*stmtIter)) {

            StmtHandle stmt = stmtIter->current();

            // calculate InActive
            retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
            usefulIter = useful->getInUsefulIterator(stmt);
            varySet = mActive->getInActiveSet(stmt);
        
            for ( ; usefulIter->isValid(); (*usefulIter)++ ) {
                 Alias::AliasTag usefulTag = usefulIter->current();
                 if(varySet->contains(usefulTag)) {
                    retval->insert(usefulTag);
                 }
            }
            mActive->copyIntoInActive(stmt, retval);


            // calculate InActive
            retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
            usefulIter = useful->getOutUsefulIterator(stmt);
            varySet = mActive->getOutActiveSet(stmt);

            for ( ; usefulIter->isValid(); (*usefulIter)++ ) {
                 Alias::AliasTag usefulTag = usefulIter->current();
                 if(varySet->contains(usefulTag)) {
                    retval->insert(usefulTag);
                 }
            }
            mActive->copyIntoOutActive(stmt, retval);
        }
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
   Data-flow set passed around on the call graph is a set of alias tags.
   The top value for this is an empty set.
*/
OA_ptr<DataFlow::DataFlowSet> ManagerICFGVaryActive::initializeTop()
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    
    return retval;
}

OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    
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
        std::cout << "-------- ManagerICFGVaryActive::meet" << std::endl;
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


/*! 
    A helper function that determines active locations, whether
    the previous stmt was active, and which memory references in the
    previous and current stmt are active
*/
OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > 
ManagerICFGVaryActive::calculateActive(
        OA_ptr<DataFlow::DFSetBitImplIterator<Alias::AliasTag> > varyIter,
        OA_ptr<DataFlow::DFSetBitImplIterator<Alias::AliasTag> > usefulIter, StmtHandle stmt)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);

    if (debug) {
        std::cout << "\tcalculateActive ---------------------" << std::endl;
    }

    // get set of active locations
    varyIter->reset();
    for ( ; varyIter->isValid(); (*varyIter)++ ) {
        Alias::AliasTag varyTag = varyIter->current();
        if (debug) { 
            std::cout << "\t\tinVary loc = ";
            std::cout << varyTag << std::endl;
        }
        usefulIter->reset();
        for ( ; usefulIter->isValid(); (*usefulIter)++ ) {
            Alias::AliasTag usefulTag = usefulIter->current();
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

//! Given an iterator over a set of assignment pairs return all the
//! may def alias tags.
OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > 
ManagerICFGVaryActive::calcDefs(OA::OA_ptr<OA::AssignPairIterator> aPairIter)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);

    if (debug) { std::cout << "In ManagerICFGVaryActive::calcDefs" << std::endl;}
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
ManagerICFGVaryActive::calcMustDefs(OA::OA_ptr<OA::AssignPairIterator> aPairIter)
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
ManagerICFGVaryActive::calcUses(OA::OA_ptr<OA::AssignPairIterator> aPairIter)
{
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);

    if (debug) { std::cout << "In ManagerICFGVaryActive::calcUses" << std::endl;}
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

void ManagerICFGVaryActive::precalcDefsAndUses(OA_ptr<ICFG::ICFGInterface> icfg)
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

  transfer function for VaryActive analysis

  // Kill off anything that is being must defined
  outVary = inVary - MustDefs[stmt]

  // Gen everything being defined if any of the uses are in vary in set
  if (inVary intersect Uses[stmt])!=empty
    outVary = outVary union Defs[stmt]
  else
    outVary = outVary

  To handle multiple assignment pairs, have the following definition 
  for MustDefs. FIXME: don't have that yet
    MustDefs = intersection_over_assign_pairs MustDefs[assign pair]

  Can and do precalculate Defs, Uses, and MustDefs for the statement.
 
  Development notes:
  OK to modify in set and return it again as result because
  solver clones the BB in sets
*/

//! OK to modify in set and return it again as result because
//! solver clones the BB in sets
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::transfer(ProcHandle proc,
                          OA_ptr<DataFlow::DataFlowSet> in, OA::StmtHandle stmt)
{   

    // ===== convert incoming set to more specific subclass =====
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > inVary;
    inVary = in.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
    // outVary = inVary
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outVary;
    outVary = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*inVary);

    if (debug) {
        std::cout << "Stmt : " << mIR->toString(stmt) << std::endl;
        std::cout << "\tinVary = ";
        inVary->dump(std::cout,mIR);
    }

    // ===== Store In Vary for the Statement =====
    // temporarily using spots that we will eventually use for
    // active alias tags
    mActive->copyIntoInActive(stmt,inVary);
    
    // Kill off anything that is being must defined
    // outVary = inVary - MustDefs[stmt]
    // note that outVary was initialized to inVary
    outVary->minusEqu(*(mStmtToMustDefs[stmt]));

    // Gen everything being defined if any of the uses are in vary in set
    // if (inVary intersect Uses[stmt])!=empty
    // inVary intersect Uses[stmt]
    inVary->intersectEqu(*(mStmtToUses[stmt]));
    if (debug) { 
        std::cout << "\n\tmStmtToUses[stmt] = ";
        mStmtToUses[stmt]->dump(std::cout,mIR);
    }
    if (! inVary->isEmpty()) {
        if (debug) {
            std::cout << "InVary has non-empty intersect with uses" 
                      << std::endl;
        }
        // outVary = outVary union Defs[stmt]
        outVary->unionEqu(*(mStmtToDefs[stmt]));
        if (debug) { 
            std::cout << "\n\tmStmtToDefs[stmt] = ";
            mStmtToDefs[stmt]->dump(std::cout,mIR);
        }
    }
    // else outVary = outVary
    // no code because indicates no change

    
    // ===== Store Out Vary for the Statement =====
    // temporarily using spots that we will eventually use for
    // active alias tags
    mActive->copyIntoOutActive(stmt,outVary);


    if (debug) {
        std::cout << "\toutVary = ";
        outVary->dump(std::cout,mIR);
    }

    return outVary;
}





//! OK to modify in set and return it again as result because
//! solver clones the BB in sets
OA_ptr<DataFlow::DataFlowSet>
ManagerICFGVaryActive::transfer(ProcHandle proc,
                        OA_ptr<DataFlow::DataFlowSet> in, OA::CallHandle call)
{

 
    //! ===== convert incoming set to more specific subclass =====
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > inVary;
    inVary = in.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outVary;
    outVary = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*inVary);

    // ===== Store In Vary for the CallHandle =====
    // temporarily using spots that we will eventually use for
    // active alias tags
    mActive->copyIntoInActive(call,inVary);
    
    // Kill off anything that is being must defined
    // outVary = inVary - MustDefs[call]
    // note that outVary was initialized to inVary
    outVary->minusEqu(*(mCallToMustDefs[call]));

    // Gen everything being defined if any of the uses are in vary in set
    // if (inVary intersect Uses[call])!=empty
    // inVary intersect Uses[call]
    inVary->intersectEqu(*(mCallToUses[call]));
    if (debug) { 
        std::cout << "\n\tmCallToUses[call] = ";
        mCallToUses[call]->dump(std::cout,mIR);
    }
    if (! inVary->isEmpty()) {
        if (debug) {
            std::cout << "InVary has non-empty intersect with uses" 
                      << std::endl;
        }
        // outVary = outVary union Defs[call]
        outVary->unionEqu(*(mCallToDefs[call]));
        if (debug) { 
            std::cout << "\n\tmCallToDefs[call] = ";
            mCallToDefs[call]->dump(std::cout,mIR);
        }
    }
    // else outVary = outVary
    // no code because indicates no change

    
    // ===== Store Out Vary for the CallHandle =====
    // temporarily using spots that we will eventually use for
    // active alias tags
    mActive->copyIntoOutActive(call,outVary);


    if (debug) {
        std::cout << "\toutVary = ";
        outVary->dump(std::cout,mIR);
    }

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
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > inRecast;
    inRecast = in.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*inRecast);

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

        retval->dump(std::cout, mIR);
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


//! Propagate a data-flow set from callee to caller
OA_ptr<DataFlow::DataFlowSet>
ManagerICFGVaryActive::calleeToCaller(ProcHandle callee,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle caller)
{
    //! ======== Just propogate entire data flow set ===========

    //! create retval as copy of dfset
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outRecast;
    outRecast = dfset.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*outRecast);
    return retval;
}


//! Propagate a data-flow set from caller to callee
OA_ptr<DataFlow::DataFlowSet> 
ManagerICFGVaryActive::callerToCallee(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{

    //! ======== Just propogate entire data flow set ============

    //! create retval as copy of dfset
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > outRecast;
    outRecast = dfset.convert<DataFlow::DFSetBitImpl<Alias::AliasTag> >();
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(*outRecast);
    return retval;
}



//! Propagate a data-flow set from call node to return node
OA_ptr<DataFlow::DataFlowSet>
ManagerICFGVaryActive::callToReturn(ProcHandle caller,
    OA_ptr<DataFlow::DataFlowSet> dfset, CallHandle call, ProcHandle callee)
{

    //! ====== Dont need to pass anything because callerToCallee will take care =====

    OA_ptr<DataFlow::DFSetBitImpl<Alias::AliasTag> > retval;
    retval = new DataFlow::DFSetBitImpl<Alias::AliasTag>(mMaxNumTags);
    return retval;
}




  } // end of namespace Activity
} // end of namespace OA
