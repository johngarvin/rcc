/*! \file
  
  \brief The AnnotationManager that generates SideEffectStandard
         which maps each procedure  to the sets of locations belonging to
         LMOD, MOD, LUSE, USE, LDEF, DEF, LREF, and REF.  
         The SideEffect analysis results also determine the set of locations
         in MOD, USE, DEF, and REF for a function call.

  \authors Michelle Strout, Priyadarshini Malusare,
           Andy Stone (alias tag update)
  \version $Id: ManagerInterSideEffectStandard.cpp,v 1.21 2005/08/08 20:03:52 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerInterSideEffectStandard.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace SideEffect {

using namespace std;
using namespace DataFlow;
using namespace CallGraph;
using namespace Alias;

static bool debug = false;

ManagerInterSideEffectStandard::ManagerInterSideEffectStandard(
  OA_ptr<InterSideEffectIRInterface> _ir)
  :
  mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerInterSideEffectStandard:ALL", debug);
    mSolver = new CallGraphDFSolver(CallGraphDFSolver::BottomUp, *this);
}

/*!
    Going to use the solve on CallGraph.  Just passing around count of tags
    in the combination of all size SideEffect sets, because if doing things 
    properly would essentially end up with six LocDFSets.  Instead, the
    callbacks will just update the InterSideEffect data-structure.
*/
OA_ptr<InterSideEffectStandard> 
ManagerInterSideEffectStandard::performAnalysis(
    OA_ptr<CallGraphInterface>  callGraph,
    OA_ptr<ParamBindings>  paramBind,
    OA_ptr<InterAliasInterface>  interAlias,
    OA_ptr<ManagerSideEffectStandard>  intraMan,
    DFPImplement  algorithm)
{
    // create a new (waiting to be filled) InterSideEffectStandard as member
    mInterSideEffect = new InterSideEffectStandard(interAlias);

    // save other things needed for analysis in member variables
    mParamBind = paramBind;
    mInterAlias = interAlias;
    mIntraMan = intraMan;

    // for all functions defined in IR create optimistic intraprocedural
    // side-effect results
    // NOTE: not using initializeNode because for this analysis we aren't
    // using the ts being passed around
    OA_ptr<NodesIteratorInterface> nodeIterPtr;
    nodeIterPtr = callGraph->getCallGraphNodesIterator();
    for( ; nodeIterPtr->isValid(); ++(*nodeIterPtr)) {
        OA_ptr<NodeInterface> node = nodeIterPtr->currentCallGraphNode();
        ProcHandle proc = node->getProc();
        if (node->isDefined()) {
            // use optimistic intra side effect analysis results
            OA_ptr<SideEffectStandard> procSideEffect;
            OA_ptr<Alias::Interface> alias;
            alias = mInterAlias->getAliasResults(proc);
            procSideEffect = new SideEffectStandard(alias);

            // empty out all the sets
            procSideEffect->emptyLMOD();
            procSideEffect->emptyMOD();
            procSideEffect->emptyLDEF();
            procSideEffect->emptyDEF();
            procSideEffect->emptyLUSE();
            procSideEffect->emptyUSE();
            procSideEffect->emptyLREF();
            procSideEffect->emptyREF();

            if(debug) {
                cout << "ManagerInterSideEffectStandard:mapProcToSideEffect:"
                     << "proc" << mIR->toString(proc) << endl;
            }

            mInterSideEffect->mapProcToSideEffect(proc, procSideEffect);
        } 
    } // loop over nodes in call graph

    // use the CallGraph dataflow solver to visit procedures iteratively
    // and solve for side-effect sets for each function call
    mSolver->solve(callGraph, algorithm);

    return mInterSideEffect;
}

//========================================================
// implementation of CallGraphDFProblemNew callbacks
//========================================================
//--------------------------------------------------------
// initialization callbacks
//--------------------------------------------------------
OA_ptr<DataFlowSet> ManagerInterSideEffectStandard::initializeTop()
{
    // Data-flow set passed around on the call graph is a CountDFSet.
    OA_ptr<CountDFSet> retval;
    retval = new CountDFSet;
    return retval;
}

OA_ptr<DataFlowSet>  
ManagerInterSideEffectStandard::initializeBottom()
{
    // Shouldn't be called.
    assert(0);

    OA_ptr<CountDFSet> retval;
    return retval;
}

//! Should generate an initial DataFlowSet for a procedure
OA_ptr<DataFlowSet> 
ManagerInterSideEffectStandard::initializeNode(ProcHandle proc)
{
    return initializeTop();
}

OA_ptr<DataFlowSet>
ManagerInterSideEffectStandard::initializeNodeIN(OA_ptr<NodeInterface> n)
{
}

OA_ptr<DataFlowSet>
ManagerInterSideEffectStandard::initializeNodeOUT(OA_ptr<NodeInterface> n)
{
}

OA_ptr<DataFlowSet> 
ManagerInterSideEffectStandard::initializeEdge(
    CallHandle call, ProcHandle caller, ProcHandle callee)
{
    // Should generate an initial DataFlowSet, use if for a call if both
    // caller and callee are defined
    return initializeTop();
}

//! Should generate an initial DataFlowSet for a call,
//! called when callee is not defined in call graph and therefore
//! doesn't have a procedure definition handle
OA_ptr<DataFlowSet> 
ManagerInterSideEffectStandard::initializeEdge(
    CallHandle call, ProcHandle caller, SymHandle callee)
{
    return initializeTop();
}


//--------------------------------------------------------
// solver callbacks
//--------------------------------------------------------

//! OK to modify set1 and return it as result, because solver
//! only passes a tempSet in as set1
OA_ptr<DataFlowSet> 
ManagerInterSideEffectStandard::meet(
    OA_ptr<DataFlowSet> set1, OA_ptr<DataFlowSet> set2)
{
    return set1->clone();
}

//! What the analysis does for the particular procedure
OA_ptr<DataFlowSet>
ManagerInterSideEffectStandard::atCallGraphNode(
    OA_ptr<DataFlowSet> inSet, 
    OA::ProcHandle proc)
{
    OA_ptr<CountDFSet> retval;

    if (debug) {
        cout << "ManagerInterSideEffectStandard::atCallGraphNode: ";
        cout << "proc = " << mIR->toString(proc) << endl;
    }

    // do intraprocedural side-effect analysis passing in 
    // interprocedural results we have so far
    OA_ptr<Alias::Interface> alias =
        mInterAlias->getAliasResults(proc);
    OA_ptr<SideEffect::SideEffectStandard> sideEffect 
        = mIntraMan->performAnalysis(proc, alias, mInterSideEffect); 
    mInterSideEffect->mapProcToSideEffect(proc,sideEffect);

    if (debug) {
      cout << "\tIntra results ---------------" << endl;
      sideEffect->dump(cout, mIR);
    }

    // count how many tags are in all sets for this procedure so we can see if
    // any are added
    int count = mInterSideEffect->getTagCount(proc);
    retval = new CountDFSet(count);

    return retval;
}


// Used when both caller and callee are defined (have associated ProcHandles)
OA_ptr<DataFlowSet>
ManagerInterSideEffectStandard::atCallGraphEdge(
    OA_ptr<DataFlowSet> inSet,
    CallHandle call,
    ProcHandle caller,
    ProcHandle callee)
{
    if (debug) {
       std::cout << "atCallGraphEdge::inSet = ";
       inSet->dump(std::cout, mIR);
    }

    if(debug) {
      std::cout << "CallHandle: " << mIR->toString(call) << std::endl; 
    }
    
    // initialize side-effect information for call to have all empty sets
    mInterSideEffect->initCallSideEffect(call);


   //! Sets the SideEffect results of the CallSites.
   OA_ptr<SideEffect::SideEffectStandard> sideEffect
      = mIR->getSideEffect(caller, mIR->getSymHandle(callee));
   mInterSideEffect->mapCallToSideEffect(call, sideEffect);



    // ----
    // Insert all tags affected in the caller into the callee:
    // ----
    OA_ptr<AliasTagIterator> tagIter;
    AliasTag tag;

    // LMOD
    tagIter = mInterSideEffect->getLMODIterator(callee);
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertLMOD(call, tag);
    }
    // MOD
    tagIter = mInterSideEffect->getMODIterator(callee);
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertMOD(call, tag);
    }
    // LDEF
    tagIter = mInterSideEffect->getLDEFIterator(callee);
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertLDEF(call, tag);
    }
    // DEF 
    tagIter = mInterSideEffect->getDEFIterator(callee);
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertDEF(call, tag);
    }
    // USE 
    tagIter = mInterSideEffect->getUSEIterator(callee);
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertUSE(call, tag);
    }
    // LUSE
    tagIter = mInterSideEffect->getLUSEIterator(callee);
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertLUSE(call, tag);
    }
    // LREF
    tagIter = mInterSideEffect->getLREFIterator(callee);
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertLREF(call, tag);
    }
    // REF
    tagIter = mInterSideEffect->getREFIterator(callee);
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertREF(call, tag);
    }

    // count how many locations are in all sets for this  procedure so we
    // can see if any are added
    // FIXME: don't have way to get count for call yet
    //int count = mInterSideEffect->getTagCount(caller);
    OA_ptr<DataFlow::CountDFSet> retval;
    retval = new DataFlow::CountDFSet();

    return retval;
}


// Used when the callee is not defined in the call graph
OA_ptr<DataFlowSet>
ManagerInterSideEffectStandard::atCallGraphEdge(
    OA_ptr<DataFlowSet> inSet, 
    CallHandle call, ProcHandle caller, SymHandle callee )
{
    if (debug) {
       std::cout << "atCallGraphEdge::inSet = ";
       inSet->dump(std::cout, mIR);
    }

    if(debug) {
      std::cout << "CallHandle: " << mIR->toString(call) << std::endl; 
    }
    
    // initialize side-effect information for call to have all empty sets
    mInterSideEffect->initCallSideEffect(call);

    // Get the callee's side effects
    OA_ptr<SideEffect::SideEffectStandard> sideEffect =
        mIR->getSideEffect(caller, callee);

    // ----
    // Insert all tags affected in the caller into the callee:
    // ----
    OA_ptr<AliasTagIterator> tagIter;
    AliasTag tag;
    
    //LMOD
    tagIter = sideEffect->getLMODIterator();
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertLMOD(call, tag);
    }

    // MOD
    tagIter = sideEffect->getMODIterator();
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertMOD(call, tag);
    }
    // LDEF
    tagIter = sideEffect->getLDEFIterator();
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertLDEF(call, tag);
    }
    // DEF
    tagIter = sideEffect->getDEFIterator();
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertDEF(call, tag);
    }
    // LUSE
    tagIter = sideEffect->getLUSEIterator();
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertLUSE(call, tag);
    }
    // USE
    tagIter = sideEffect->getUSEIterator();
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertUSE(call, tag);
    }
    // LREF
    tagIter = sideEffect->getLREFIterator();
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertLREF(call, tag);
    }
    // REF
    tagIter = sideEffect->getREFIterator();
    for ( ; tagIter->isValid(); ++(*tagIter) ) {
      tag = tagIter->current();
      mInterSideEffect->insertREF(call, tag);
    }
    
    // count how many locations are in all sets for this  procedure so we
    // can see if any are added
    // FIXME: don't have way to get count for call yet
    //int count = mInterSideEffect->getTagCount(caller);
    OA_ptr<DataFlow::CountDFSet> retval;
    retval = new DataFlow::CountDFSet();

    return retval;
}

  } // end of namespace SideEffect
} // end of namespace OA
