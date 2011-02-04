/*! \file
  
  \brief Declarations of the AnnotationManager that generates ReachConsts
         analysis results for each procedure interprocedurally

  \authors Michelle Strout, Barbara Kreaseck
  \version $Id: ManagerInterReachConsts.hpp,v 1.2 2005/06/10 02:32:04 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerInterReachConsts_h
#define ManagerInterReachConsts_h

//--------------------------------------------------------------------
// OpenAnalysis headers

//#include <OpenAnalysis/Activity/ManagerActiveStandard.hpp>
//#include <OpenAnalysis/Activity/ManagerInterDep.hpp>
//#include <OpenAnalysis/Activity/ManagerInterUseful.hpp>
//#include <OpenAnalysis/Activity/ManagerInterVary.hpp>
#include <OpenAnalysis/IRInterface/ReachConstsIRInterface.hpp>

//#include <OpenAnalysis/Activity/InterActive.hpp>
#include <OpenAnalysis/CFG/EachCFGInterface.hpp>
#include <OpenAnalysis/Alias/InterAliasInterface.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/ReachConsts/Interface.hpp>
#include <OpenAnalysis/ReachConsts/ManagerReachConstsStandard.hpp>
#include <OpenAnalysis/ReachConsts/InterReachConsts.hpp>
#include <OpenAnalysis/CallGraph/CallGraphInterface.hpp>
#include <OpenAnalysis/CallGraph/CallGraph.hpp>

namespace OA {
  namespace ReachConsts {


/*! 
   Generates InterReachConsts.
*/
class ManagerInterReachConsts {
public:
  ManagerInterReachConsts(OA_ptr<ReachConstsIRInterface> _ir);
  ~ManagerInterReachConsts () {}

  OA_ptr<InterReachConsts> performAnalysis(
          OA_ptr<CallGraph::CallGraphInterface> callGraph,
          // OA_ptr<DataFlow::ParamBindings> paramBind,
          OA_ptr<Alias::InterAliasInterface> interAlias,
          OA_ptr<SideEffect::InterSideEffectInterface> interSE,
          OA_ptr<CFG::EachCFGInterface> eachCFG,
          DataFlow::DFPImplement algorithm);

private: // member variables

  OA_ptr<ReachConstsIRInterface> mIR;
};

  } // end of ReachConsts namespace
} // end of OA namespace

#endif
