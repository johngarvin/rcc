/*! \file
  
  \brief The AnnotationManager that generates InterReachConsts
  INTERprocedurally.

  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: ManagerInterReachConsts.cpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerInterReachConsts.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace ReachConsts {

static bool debug = false;

ManagerInterReachConsts::ManagerInterReachConsts(
    OA_ptr<ReachConstsIRInterface> _ir) : mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerInterReachConsts:ALL", debug);
}

/*!
   Just calls ManagerReachConstStandard::performAnalysis on each procedure
*/
OA_ptr<InterReachConsts> 
ManagerInterReachConsts::performAnalysis(
        OA_ptr<CallGraph::CallGraphInterface> callGraph,
        // OA_ptr<DataFlow::ParamBindings> paramBind,
        OA_ptr<Alias::InterAliasInterface> interAlias,
        OA_ptr<SideEffect::InterSideEffectInterface> interSE,
        OA_ptr<CFG::EachCFGInterface> eachCFG,
        DataFlow::DFPImplement algorithm)
{
  OA_ptr<InterReachConsts> retval;
  retval = new InterReachConsts;


  // Iterate over the procedures in the call graph
  OA_ptr<CallGraph::NodesIteratorInterface> procIter
      = callGraph->getCallGraphNodesIterator();
  for ( ; procIter->isValid(); ++(*procIter)) { 
   
    OA_ptr<CallGraph::NodeInterface> Node = procIter->currentCallGraphNode();
    ProcHandle proc = Node->getProc();

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
    OA_ptr<CFG::CFGInterface> cfg;
    cfg = eachCFG->getCFGResults(proc);

    // create ReachConsts manager
    OA_ptr<ManagerReachConstsStandard> rcman;
    rcman = new ReachConsts::ManagerReachConstsStandard(mIR);

    OA_ptr<ReachConsts::ReachConstsStandard> rcs 
        = rcman->performAnalysis(proc, cfg,
                alias,interSE,algorithm);

    // put reachconsts results in InterReachConsts
    retval->mapProcToReachConsts(proc,rcs);

  }
 
  return retval;
}
 

  } // end of namespace Activity
} // end of namespace OA
