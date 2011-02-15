/*! \file
  
  \brief The AnnotationManager that generates InterActive INTERprocedurally.

  \authors Michelle Strout
  \version $Id: ManagerInterActive.cpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerInterActive.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Activity {

static bool debug = false;

ManagerInterActive::ManagerInterActive(
    OA_ptr<Activity::ActivityIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerInterActive:ALL", debug);
}

/*!
   Just calls ManagerActiveStandard::performAnalysis on each procedure
*/
OA_ptr<InterActive> 
ManagerInterActive::performAnalysis(
        OA_ptr<CallGraph::Interface> callGraph,
        OA_ptr<DataFlow::ParamBindings> paramBind,
        OA_ptr<Alias::InterAliasInterface> interAlias,
        OA_ptr<SideEffect::InterSideEffectInterface> interSE,
        OA_ptr<CFG::EachCFGInterface> eachCFG)
{
  assert(0); // currently not working, ManagerICFGActive is better approach
             // and therefore haven't updated this one with fixes

  OA_ptr<InterActive> retval;
  retval = new InterActive;

  // create a Manager that generates interprocedure dep information
  OA_ptr<ManagerInterDep> depman;
  depman = new ManagerInterDep(mIR);
  OA_ptr<InterDep> interDep = depman->performAnalysis(callGraph, paramBind,
          interAlias, interSE, eachCFG);

  if (debug) { interDep->dump(std::cout, mIR); }

  // based on the dep information, ManagerInterVary will query the 
  // subroutine for what locations are indep or get that information
  // from other routines in the call graph
  OA_ptr<ManagerInterVary> varyman;
  varyman = new ManagerInterVary(mIR);
  OA_ptr<InterVary> interVary = varyman->performAnalysis(callGraph, paramBind,
          interAlias, eachCFG, interDep);
  
  if (debug) { interVary->dump(std::cout, mIR); }
  
  // based on the dep information, ManagerInterUseful will query the 
  // subroutine for what locations are dep or get that information
  // from other routines in the call graph
  OA_ptr<ManagerInterUseful> usefulman;
  usefulman = new ManagerInterUseful(mIR);
  OA_ptr<InterUseful> interUseful = usefulman->performAnalysis(callGraph, 
          paramBind, interAlias, eachCFG, interDep);

  if (debug) { interUseful->dump(std::cout, mIR); }

  // Iterate over the procedures in the call graph
  OA_ptr<CallGraph::Interface::NodesIterator> procIter
      = callGraph->getNodesIterator();
  for ( ; procIter->isValid(); ++(*procIter)) { 
    
    ProcHandle proc = procIter->current()->getProc();

    // if this procedure isn't defined then move on
    if (proc==ProcHandle(0)) { 
        continue;
    }

    //set currentProc()
    //mIR->currentProc(proc);

    // get Alias::Interface for this proc
    OA_ptr<Alias::Interface> alias;
    alias = interAlias->getAliasResults(proc);

    // get CFG for this proc
    OA_ptr<CFG::Interface> cfg;
    cfg = eachCFG->getCFGResults(proc);

    // get useful for this proc
    OA_ptr<UsefulStandard> useful = interUseful->getUsefulResults(proc);

    // get vary for this proc
    OA_ptr<VaryStandard> vary = interVary->getVaryResults(proc);

    // create Active manager
    OA_ptr<Activity::ManagerActiveStandard> activeman;
    activeman = new Activity::ManagerActiveStandard(mIR);

    OA_ptr<ActiveStandard> active 
        = activeman->performAnalysis(proc, cfg,
                alias,vary, useful);
    if (debug) { active->dump(std::cout, mIR); }

    // put activity results in InterMPICFGActivity
    retval->mapProcToActive(proc,active);

  }

  // iterate over all symbols to determine the size
  OA_ptr<SymHandleIterator> symIter = retval->getActiveSymIterator();
  int bytes = 0;
  for ( ; symIter->isValid(); (*symIter)++ ) {
      SymHandle sym = symIter->current();

      bytes += mIR->getSizeInBytes(sym);

      if (debug) {
          std::cout << "ManagerInterActive: sym = " << mIR->toString(sym)
                    << ", size = " << mIR->getSizeInBytes(sym) << ", bytes = "
                    << bytes << std::endl;
      }
  }
  retval->setActiveSizeInBytes(bytes);
 
  return retval;
}
 

  } // end of namespace Activity
} // end of namespace OA
