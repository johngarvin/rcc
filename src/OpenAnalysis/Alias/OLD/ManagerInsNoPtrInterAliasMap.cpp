/*! \file
  
NOTE: deprecated!!  No longer being compiled.
6/2/06, Removed ManagerInsNoPtrInterAliasMap because it is no longer
valid.  It used the CallGraph data-flow analysis framework
and dependended on the isRefParam() call, which has been deprecated.

  \brief The AnnotationManager that generates InterAliasMaps in a context
         and flow insensitive way and handles pointers conservatively.

  \authors Michelle Strout, Brian White
  \version $Id: ManagerInsNoPtrInterAliasMap.cpp,v 1.2.6.1 2006/01/18 23:28:42 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerInsNoPtrInterAliasMap.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Alias {

static bool debug = false;

/*!
*/
ManagerInsNoPtrInterAliasMap::ManagerInsNoPtrInterAliasMap(
  OA_ptr<AliasIRInterface> _ir) 
    : DataFlow::CallGraphDFProblemNew(DataFlow::CallGraphDFProblemNew::TopDown),
      mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerInsNoPtrInterAliasMap:ALL", debug);
}

OA_ptr<Alias::InterAliasMap> 
ManagerInsNoPtrInterAliasMap::performAnalysis(
        OA_ptr<CallGraph::Interface> callGraph,
        OA_ptr<DataFlow::ParamBindings> paramBind)
{
  // store results that will be needed in callbacks
  mParamBind = paramBind;

  // create an empty InterAliasMap
  OA_ptr<ManagerAliasMapBasic> manAM; manAM = new ManagerAliasMapBasic(mIR);
  mInterAliasMap = new InterAliasMap(manAM);

  // call iterative data-flow solver for CallGraph
  DataFlow::CallGraphDFProblemNew::solve(callGraph);
    
  return mInterAliasMap;
}

//========================================================
// implementation of CallGraphDFProblemNew callbacks
//========================================================
//--------------------------------------------------------
// initialization callbacks
//--------------------------------------------------------

/*!
   Data-flow set passed around on the call graph is an
   SymAliasSets.  The top value for this is that all
   Symbols are in own set,
   which is essentially the default assumption.
*/
OA_ptr<DataFlow::DataFlowSet> ManagerInsNoPtrInterAliasMap::initializeTop()
{
    OA_ptr<SymAliasSets> retval;
    retval = new SymAliasSets;
    return retval;
}

/*! Shouldn't be called.
 */
OA_ptr<DataFlow::DataFlowSet>  
ManagerInsNoPtrInterAliasMap::initializeBottom()
{
    assert(0);
    // have different bottom based on what procedure we are
    /*
    OA_ptr<ManagerSymAliasSetsBottom> man;
    man = new ManagerSymAliasSetsBottom(mIR);

    OA_ptr<SymAliasSets> retval = man->performAnalysis(mProc);
    */
    OA_ptr<SymAliasSets> retval;
    return retval;
}

//! Should generate an initial DataFlowSet for a procedure
OA_ptr<DataFlow::DataFlowSet> 
ManagerInsNoPtrInterAliasMap::initializeNode(ProcHandle proc)
{
    OA_ptr<ManagerSymAliasSetsTop> man;
    man = new ManagerSymAliasSetsTop(mIR);

    OA_ptr<SymAliasSets> retval = man->performAnalysis(proc);
    return retval;
}

//! Should generate an initial DataFlowSet, use if for a call if both caller
//! and callee are defined
OA_ptr<DataFlow::DataFlowSet> 
ManagerInsNoPtrInterAliasMap::initializeEdge(ExprHandle call, 
                                                 ProcHandle caller,
                                                 ProcHandle callee)
{
    // FIXME: do we even use this DataFlowSet?
    OA_ptr<ManagerSymAliasSetsTop> man;
    man = new ManagerSymAliasSetsTop(mIR);

    OA_ptr<SymAliasSets> retval = man->performAnalysis(caller);
    return retval;
}

//! Should generate an initial DataFlowSet for a call,
//! called when callee is not defined in call graph and therefore
//! doesn't have a procedure definition handle
OA_ptr<DataFlow::DataFlowSet> 
ManagerInsNoPtrInterAliasMap::initializeEdge(ExprHandle call, 
                                                 ProcHandle caller,
                                                 SymHandle callee)
{
    // FIXME: do we even use this DataFlowSet?
    OA_ptr<ManagerSymAliasSetsTop> man;
    man = new ManagerSymAliasSetsTop(mIR);

    OA_ptr<SymAliasSets> retval = man->performAnalysis(caller);
    return retval;
}

//--------------------------------------------------------
// solver callbacks 
//--------------------------------------------------------
  
//! OK to modify set1 and return it as result, because solver
//! only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> 
ManagerInsNoPtrInterAliasMap::meet(OA_ptr<DataFlow::DataFlowSet> set1, 
                                       OA_ptr<DataFlow::DataFlowSet> set2)
{
    if (debug) {
        std::cout << "ManagerInsNoPtrInterAliasMap::meet" << std::endl;
    }
    OA_ptr<SymAliasSets> remapSet1 = set1.convert<SymAliasSets>();
    OA_ptr<SymAliasSets> remapSet2 = set2.convert<SymAliasSets>();
    if (debug) {
        std::cout << "\tremapSet1 = ";
        //remapSet1->dump(std::cout, mIR);
        remapSet1->dump(std::cout);
        std::cout << "\tremapSet2 = ";
        //remapSet2->dump(std::cout, mIR);
        remapSet2->dump(std::cout);
    }
    OA_ptr<SymAliasSets> retval = remapSet1->meet(*remapSet2);
    if (debug) {
        std::cout << "\tretval = ";
        retval->dump(std::cout);
    }
    return retval;
}

//! What the analysis does for the particular procedure
OA_ptr<DataFlow::DataFlowSet>
ManagerInsNoPtrInterAliasMap::atCallGraphNode(
        OA_ptr<DataFlow::DataFlowSet> inSet, 
        OA::ProcHandle proc)
{
    if (debug) {
        std::cout << "In ManagerInsNoPtrInterAliasMap::atCallGraphNode" 
                  << std::endl;
    }

    // cast to more specific data-flow set data type
    OA_ptr<SymAliasSets> symSets = inSet.convert<SymAliasSets>();

    ////// Using the input SymAliasSet create a seed AliasMap
    // create AliasMap
    OA_ptr<AliasMap> seedAliasMap;
    seedAliasMap = new AliasMap(proc);

    // iterate over formal parameters for this procedure
    OA_ptr<SymHandleIterator> symIter = mParamBind->getFormalIterator(proc);
    for ( ; symIter->isValid(); (*symIter)++ ) {
        SymHandle formal = symIter->current();
        if (debug) { 
            std::cout << "\tformal = " << mIR->toString(formal) << std::endl; 
        }
        
        // if it is a reference param
        if (mParamBind->isRefParam(formal)) {
            // get the location for this formal
            OA_ptr<Location> formalLoc = mIR->getLocation(proc,formal);
            if (debug) { 
                std::cout << "\tformalLoc = ";
                formalLoc->dump(std::cout,mIR);
                std::cout << std::endl;
                std::cout << "\tSymAliasSets = ";
                //symSets->dump(std::cout, mIR);
                symSets->dump(std::cout);
            }

            // get all the symbols that this formal aliases
            OA_ptr<SymHandleIterator> aliasIter 
                = symSets->getSymIterator(formal);
            for ( ; aliasIter->isValid(); (*aliasIter)++ ) {
                SymHandle otherSym = aliasIter->current();
                if (debug) { 
                  std::cout << "\t\tother = " << mIR->toString(otherSym);
                  std::cout << ", hval = " << otherSym.hval() 
                            << std::endl; 
                }
                // get the location for each symbol
                OA_ptr<Location> otherLoc = mIR->getLocation(proc,otherSym);
                // alias the locations in the seedAliasMap
                seedAliasMap->aliasLocs(formalLoc,otherLoc);
            }
        }
    }
                
    ////// call Manager that creates an AliasMap for the procedure
    ////// given the seed AliasMap
    OA_ptr<ManagerAliasMapBasic> aliasMan;
    aliasMan = new ManagerAliasMapBasic(mIR);
    assert(0); // this whole thing doesn't work any more because if isRefParam
    //OA_ptr<AliasMap> alias = aliasMan->performAnalysis(proc,seedAliasMap);
    OA_ptr<AliasMap> alias;
    mInterAliasMap->mapProcToAliasMap(proc,alias);

    // return inSets, didn't change it
    return inSet;
}

//! What the analysis does for a particular call
OA_ptr<DataFlow::DataFlowSet>
ManagerInsNoPtrInterAliasMap::atCallGraphEdge(
        OA_ptr<DataFlow::DataFlowSet> inSet, 
        OA::ExprHandle call, ProcHandle caller, ProcHandle callee)
{
    if (debug) {
        std::cout << "atCallGraphEdge::inSet = ";
        inSet->dump(std::cout);
    }
    // pass on inSet
    return inSet;
}

//! translate results from procedure node to callee edge if top-down
//! or to caller edge if bottom-up
OA_ptr<DataFlow::DataFlowSet>  
ManagerInsNoPtrInterAliasMap::nodeToEdge(ProcHandle proc, 
        OA_ptr<DataFlow::DataFlowSet> procDFSet, ExprHandle call)
{
  if (debug) {
      std::cout << "In ManagerInsNoPtrInterAliasMap::nodeToEdge" << std::endl;
  }

  // create output DataFlowSet
  OA_ptr<SymAliasSets> retval;
  retval = new SymAliasSets;

  // alias results for the caller
  OA_ptr<Alias::Interface> alias = mInterAliasMap->getAliasResults(proc);

  // loop over callsiteparams
  OA_ptr<MemRefHandleIterator> actIter1 = mParamBind->getActualIterator(call);
  OA_ptr<MemRefHandleIterator> actIter2 = mParamBind->getActualIterator(call);
  for ( ; actIter1->isValid(); (*actIter1)++ ) {
    MemRefHandle act1 = actIter1->current();
    SymHandle formal1 = mParamBind->getCalleeFormal(call, act1);
    if (debug) {
        std::cout << "\tact1 = " << mIR->toString(act1) << std::endl;
        std::cout << "\tformal1 = " << mIR->toString(formal1) << std::endl;
    }

    for ( ; actIter2->isValid(); (*actIter2)++ ) {
      MemRefHandle act2 = actIter2->current();
      SymHandle formal2 = mParamBind->getCalleeFormal(call, act2);
      if (debug) {
          std::cout << "\tact2 = " << mIR->toString(act2) << std::endl;
          std::cout << "\tformal2 = " << mIR->toString(formal2) << std::endl;
      }

      // if both formals are reference params
      // do pairwise compare
      // if the actuals may overlap then put formals in same alias set
      // in SymAliasSet
      if ( mParamBind->isRefParam(formal1) && mParamBind->isRefParam(formal2)
           && alias->alias(act1,act2) != NOALIAS ) 
      {
        if (debug) { std::cout << "\tact1 and act2 alias" << std::endl; }
        retval->mergeSyms(formal1, formal2);
      }
    }

    // if actual aliases can refer to any non-local locations then
    // add the symbols for those locations into the same set in SymAliasSet
    if (mParamBind->isRefParam(formal1)) {
      OA_ptr<LocIterator> locIter = alias->getMayLocs(act1);
      for ( ; locIter->isValid(); (*locIter)++ ) {
        OA_ptr<Location> loc = locIter->current();
        if (!loc->isLocal()  && loc->isaNamed() ) {
          OA_ptr<NamedLoc> nloc = loc.convert<NamedLoc>();
          retval->mergeSyms(formal1, nloc->getSymHandle());
        }
      }
    }

  }

  return retval;
    
}

//! translate results from caller edge to procedure node if top-down
//! or from callee edge if bottom-up
OA_ptr<DataFlow::DataFlowSet>  
ManagerInsNoPtrInterAliasMap::edgeToNode(ExprHandle call, 
        OA_ptr<DataFlow::DataFlowSet> callDFSet, ProcHandle proc)
{
    if (debug) {
        std::cout << "edgeToNode::callDFSet = ";
        callDFSet->dump(std::cout);
    }
    return callDFSet;
}



  } // end of namespace Alias
} // end of namespace OA
