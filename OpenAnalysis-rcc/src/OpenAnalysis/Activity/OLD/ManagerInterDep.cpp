/*! \file
  
  \brief The AnnotationManager that generates InterDep.

  \authors Michelle Strout
  \version $Id: ManagerInterDep.cpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerInterDep.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Activity {

static bool debug = false;

/*!
*/
ManagerInterDep::ManagerInterDep(
  OA_ptr<ActivityIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerInterDep:ALL", debug);
    mSolver = new DataFlow::CallGraphDFSolver(DataFlow::CallGraphDFSolver::BottomUp,*this);
}

OA_ptr<Activity::InterDep> 
ManagerInterDep::performAnalysis(
        OA_ptr<CallGraph::CallGraphInterface> callGraph,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        OA_ptr<Alias::InterAliasInterface> interAlias,
        OA_ptr<SideEffect::InterSideEffectInterface> interSE,
        OA_ptr<CFG::EachCFGInterface> eachCFG,
        DataFlow::DFPImplement algorithm)
{
  // store results that will be needed in callbacks
  mParamBind = paramBind;
  mInterAlias = interAlias;
  mInterSE = interSE;
  mEachCFG = eachCFG;

  // create an empty InterDep
  mInterDep = new InterDep();

  // call iterative data-flow solver for CallGraph
  mSolver->solve(callGraph,algorithm);
  
  return mInterDep;
}

//========================================================
// implementation of CallGraphDFProblemNew callbacks
//========================================================
//--------------------------------------------------------
// initialization callbacks
//--------------------------------------------------------

/*!
   Data-flow set passed around on the call graph is an
   DepDFSet.  The top value for this is no uses mapping to
   any defs and vice versa.
*/
OA_ptr<DataFlow::DataFlowSet> ManagerInterDep::initializeTop()
{
    OA_ptr<DepDFSet> retval;
    retval = new DepDFSet;
    return retval;
}

/*! Shouldn't be called.
 */
OA_ptr<DataFlow::DataFlowSet>  
ManagerInterDep::initializeBottom()
{
    assert(0);
    // have different bottom based on what procedure we are
    // specificallly have to say all defs depend on all uses

    OA_ptr<DepDFSet> retval;
    retval = new DepDFSet;
    return retval;
}

//! Should generate an initial DataFlowSet for a procedure
OA_ptr<DataFlow::DataFlowSet> 
ManagerInterDep::initializeNode(ProcHandle proc)
{
    return initializeTop();
}

//! Should generate an initial DataFlowSet, use if for a call if both caller
//! and callee are defined
OA_ptr<DataFlow::DataFlowSet> 
ManagerInterDep::initializeEdge(CallHandle call, 
                                                 ProcHandle caller,
                                                 ProcHandle callee)
{
    return initializeTop();
}

//! Should generate an initial DataFlowSet for a call,
//! called when callee is not defined in call graph and therefore
//! doesn't have a procedure definition handle
OA_ptr<DataFlow::DataFlowSet> 
ManagerInterDep::initializeEdge(CallHandle call, 
                                                 ProcHandle caller,
                                                 SymHandle callee)
{
    return initializeTop();
}

//--------------------------------------------------------
// solver callbacks 
//--------------------------------------------------------
  
//! OK to modify set1 and return it as result, because solver
//! only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> 
ManagerInterDep::meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                      OA_ptr<DataFlow::DataFlowSet> set2)
{
    if (debug) {
        std::cout << "ManagerInterDep::meet" << std::endl;
    }
    OA_ptr<DepDFSet> remapSet1 = set1.convert<DepDFSet>();
    OA_ptr<DepDFSet> remapSet2 = set2.convert<DepDFSet>();
    if (debug) {
        std::cout << "\tremapSet1 = ";
        remapSet1->dump(std::cout, mIR);
        std::cout << "\tremapSet2 = ";
        remapSet2->dump(std::cout, mIR);
    }

    // making sure we don't trounce set1 just in case
    OA_ptr<DepDFSet> retval;
    OA_ptr<DataFlow::DataFlowSet> temp = remapSet1->clone();
    retval = temp.convert<DepDFSet>();
    *retval = retval->setUnion(*remapSet2);
    if (debug) {
        std::cout << "\tretval = ";
        OA_ptr<DepDFSet> temp = retval.convert<DepDFSet>();
        temp->dump(std::cout, mIR);
    }
    return retval;
}

//! What the analysis does for the particular procedure
OA_ptr<DataFlow::DataFlowSet>
ManagerInterDep::atCallGraphNode(
        OA_ptr<DataFlow::DataFlowSet> inSet, 
        OA::ProcHandle proc)
{
    if (debug) {
        std::cout << "In ManagerInterDep::atCallGraphNode " 
                  << mIR->toString(proc) << std::endl;
    }

    // create empty depDFSet for this procedure
    OA_ptr<DepDFSet> retval; retval = new DepDFSet;

    // get alias results for this procedure
    OA_ptr<Alias::Interface> alias = mInterAlias->getAliasResults(proc);
    
    // create empty Dep results for this procedure
    OA_ptr<ManagerDepStandard> depman;
    depman = new ManagerDepStandard(mIR);
    OA_ptr<DepStandard> depResults 
        = depman->performAnalysis(proc, alias,mEachCFG->getCFGResults(proc),
                                  mInterDep, mParamBind,
                                  DataFlow::ITERATIVE);

    // assign dep results to this procedure
    mInterDep->mapProcToDep(proc,depResults);

    // resulting DepDFSet for whole procedure will be sent to each
    // of the call edges
    if (debug) {
        std::cout << "ManagerInterDep::atCallGraphNode, returning ";
        depResults->getFinalDep()->dump(std::cout,mIR);
    }
    return depResults->getFinalDep();
}

//! What the analysis does for a particular call
//! Use this version if callee is defined
OA_ptr<DataFlow::DataFlowSet>
ManagerInterDep::atCallGraphEdge(
        OA_ptr<DataFlow::DataFlowSet> inSet, 
        CallHandle call, ProcHandle caller, ProcHandle callee )
{
   if (debug) {
       std::cout << "atCallGraphEdge::inSet = ";
       inSet->dump(std::cout, mIR);
   }
	
   // cast DataFlowSet to more specific subclass
   OA_ptr<DepDFSet> recastInSet = inSet.convert<DepDFSet>();

   // create a new DepDFSet that is empty
   OA_ptr<DepDFSet> retval;
   retval = new DepDFSet;

   assert(callee != ProcHandle(0));

   // iterate over the deps for the call
   OA_ptr<DepIterator> depIter = recastInSet->getDepIterator();
   for ( ; depIter->isValid(); (*depIter)++ ) {
     OA_ptr<Location> useLoc = depIter->use();
     OA_ptr<Location> defLoc = depIter->def();
     if (debug) {
       std::cout << "\tuseLoc = ";
       useLoc->dump(std::cout,mIR);
       std::cout << std::endl;
       std::cout << "\tdefLoc = ";
       defLoc->dump(std::cout,mIR);
       std::cout << std::endl;
     }

     // see if either of these locations are part of implicit
     // deps that have been explicitly removed
     bool useLocImplicitRemove = false;
     bool defLocImplicitRemove = false;
     if (recastInSet->isImplicitRemoved(useLoc)) {
       useLocImplicitRemove = true;
     }
     if (recastInSet->isImplicitRemoved(defLoc)) {
       defLocImplicitRemove = true;
     }

     // have visitor convert both of these to may aliases 
     // in caller, if get null location then there is no
     // equivalent location in the caller
     OA_ptr<DataFlow::CalleeToCallerVisitor> useVisitor; 
     useVisitor = new DataFlow::CalleeToCallerVisitor(callee, call, 
						      caller, mInterAlias, mParamBind, mIR );
     useLoc->acceptVisitor(*useVisitor);
     OA_ptr<LocIterator> useIter = useVisitor->getCallerLocIterator();

     OA_ptr<DataFlow::CalleeToCallerVisitor> defVisitor;
     defVisitor = new DataFlow::CalleeToCallerVisitor(callee, call, 
						      caller, mInterAlias, mParamBind, mIR);
     defLoc->acceptVisitor(*defVisitor);
     OA_ptr<LocIterator> defIter = defVisitor->getCallerLocIterator();

     // insert these deps into retval
     for (useIter->reset() ; useIter->isValid(); (*useIter)++ ) {
       // remove implicit pair if necessary
       if (useLocImplicitRemove==true) {
	 retval->removeImplicitDep(useIter->current(),
				   useIter->current());
       }
       for (defIter->reset(); defIter->isValid(); (*defIter)++ ) {
	 retval->insertDep(useIter->current(), defIter->current());
	 if (debug) {
	   std::cout << "Inserting into retval, use = ";
	   useIter->current()->dump(std::cout,mIR);
	   std::cout << "Inserting into retval, def = ";
	   defIter->current()->dump(std::cout,mIR);
	 }

	 // remove implicit pair if necessary
	 if (defLocImplicitRemove==true) {
	   retval->removeImplicitDep(defIter->current(),
				     defIter->current());
	 }
       }
     }

   } 
   
   // the generated DepDFSet must be associated to the specific call
   // to be of any use
   mInterDep->mapCallToDep(call, retval);
   
   return retval;
}


//! What the analysis does for a particular call
//! Use this version when callee is not defined in the program (i.e. sin, cos)
OA_ptr<DataFlow::DataFlowSet>
ManagerInterDep::atCallGraphEdge(
        OA_ptr<DataFlow::DataFlowSet> inSet, 
        CallHandle call, ProcHandle caller, SymHandle callee )
{
  if (debug) {
    std::cout << "atCallGraphEdge::inSet = ";
    inSet->dump(std::cout, mIR);
  }
  
  // cast DataFlowSet to more specific subclass
  OA_ptr<DepDFSet> recastInSet = inSet.convert<DepDFSet>();
  
  // create a new DepDFSet that is empty
  OA_ptr<DepDFSet> retval;
  retval = new DepDFSet;
  
  // if callee node is undefined then estimate Dep with side-effect results
  // FIXME: no way to pass up must defs, but this will still give us
  // a conservative estimate
  if (debug) { std::cout << "\tcallee is undefined" << std::endl; }
  // get side-effect MOD locations and USE locations
  // and make dep pairs
  OA_ptr<LocIterator> useIter = mInterSE->getUSEIterator(call);
  OA_ptr<LocIterator> modIter = mInterSE->getMODIterator(call);
  for (modIter->reset(); modIter->isValid(); (*modIter)++) {
    retval->insertDep(useIter->current(), modIter->current());
    if (debug) {
      std::cout << "\t\tuseIter->current = ";
      useIter->current()->dump(std::cout,mIR);
      std::cout << "\t\tmodIter->current = ";
      modIter->current()->dump(std::cout,mIR);
    }
  }
  
  // the generated DepDFSet must be associated to the specific call
  // to be of any use
  mInterDep->mapCallToDep(call, retval);
  
  return retval;
}



OA_ptr<DataFlow::DataFlowSet>  
ManagerInterDep::nodeToEdge(ProcHandle proc, 
        OA_ptr<DataFlow::DataFlowSet> procDFSet, CallHandle call)
{
  if (debug) {
      std::cout << "In ManagerInterDep::nodeToEdge" << std::endl;
  }

  return procDFSet;
}

//! translate results from caller edge to procedure node if top-down
//! or from callee edge if bottom-up
OA_ptr<DataFlow::DataFlowSet>  
ManagerInterDep::edgeToNode(CallHandle call, 
        OA_ptr<DataFlow::DataFlowSet> callDFSet, ProcHandle proc)
{
    if (debug) {
        std::cout << "edgeToNode::callDFSet = ";
        callDFSet->dump(std::cout, mIR);
    }
    return callDFSet;
   /* 
    // cast DataFlowSet to more specific subclass
    OA_ptr<DepDFSet> recastCallDFSet = callDFSet.convert<DepDFSet>();

    // create a new DepDFSet that is empty
    OA_ptr<DepDFSet> retval;
    retval = new DepDFSet;
    
    // iterate over the deps for the call
    OA_ptr<DepIterator> depIter = recastCallDFSet->getDepIterator();
    for ( ; depIter->isValid(); (*depIter)++ ) {
        OA_ptr<Location> useLoc = depIter->use();
        OA_ptr<Location> defLoc = depIter->def();
        if (debug) {
            std::cout << "\tuseLoc = ";
            useLoc->dump(std::cout,mIR);
            std::cout << std::endl;
            std::cout << "\tdefLoc = ";
            defLoc->dump(std::cout,mIR);
            std::cout << std::endl;
        }

        // see if either of these locations are part of implicit
        // deps that have been explicitly removed
        bool useLocImplicitRemove = false;
        bool defLocImplicitRemove = false;
        if (recastCallDFSet->isImplicitRemoved(useLoc)) {
            useLocImplicitRemove = true;
        }
        if (recastCallDFSet->isImplicitRemoved(defLoc)) {
            defLocImplicitRemove = true;
        }

        // have visitor convert both of these to may aliases 
        // in caller, if get null location then there is no
        // equivalent location in the caller
        OA_ptr<DataFlow::CalleeToCallerVisitor> useVisitor; 
        useVisitor = new DataFlow::CalleeToCallerVisitor(call, 
                proc, mInterAlias, mParamBind);
        useLoc->acceptVisitor(*useVisitor);
        OA_ptr<LocIterator> useIter = useVisitor->getCallerLocIterator();

        OA_ptr<DataFlow::CalleeToCallerVisitor> defVisitor;
        defVisitor = new DataFlow::CalleeToCallerVisitor(call, 
                proc, mInterAlias, mParamBind);
        defLoc->acceptVisitor(*defVisitor);
        OA_ptr<LocIterator> defIter = defVisitor->getCallerLocIterator();

        // insert these deps into retval
        for (useIter->reset() ; useIter->isValid(); (*useIter)++ ) {
            // remove implicit pair if necessary
            if (useLocImplicitRemove==true) {
                retval->removeImplicitDep(useIter->current(),
                                          useIter->current());
            }
            for (defIter->reset(); defIter->isValid(); (*defIter)++ ) {
                retval->insertDep(useIter->current(), defIter->current());
                if (debug) {
                    std::cout << "Inserting into retval, use = ";
                    useIter->current()->dump(std::cout,mIR);
                    std::cout << "Inserting into retval, def = ";
                    defIter->current()->dump(std::cout,mIR);
                }

                // remove implicit pair if necessary
                if (defLocImplicitRemove==true) {
                    retval->removeImplicitDep(defIter->current(),
                                              defIter->current());
                }
            }
        }

    } 

    // the generated DepDFSet must be associated to the specific call
    // to be of any use
    mInterDep->mapCallToDep(call, retval);

    return retval;
    */
}



  } // end of namespace Activity
} // end of namespace OA
