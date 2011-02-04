/*! \file
  
  \brief Declarations of the AnnotationManager that generates Activity
         analysis results for each procedure interprocedurally

  \authors Michelle Strout
  \version $Id: ManagerInterActive.hpp,v 1.2 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerInterActive_h
#define ManagerInterActive_h

//--------------------------------------------------------------------
// OpenAnalysis headers

#include <OpenAnalysis/Activity/ManagerActiveStandard.hpp>
#include <OpenAnalysis/Activity/ManagerInterDep.hpp>
#include <OpenAnalysis/Activity/ManagerInterUseful.hpp>
#include <OpenAnalysis/Activity/ManagerInterVary.hpp>
#include <OpenAnalysis/IRInterface/ActivityIRInterface.hpp>

#include <OpenAnalysis/Activity/InterActive.hpp>
#include <OpenAnalysis/CFG/EachCFGInterface.hpp>
#include <OpenAnalysis/Alias/InterAliasInterface.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>

namespace OA {
  namespace Activity {


/*! 
   Generates InterActive.
*/
class ManagerInterActive {
public:
  ManagerInterActive(OA_ptr<Activity::ActivityIRInterface> _ir);
  ~ManagerInterActive () {}

  OA_ptr<InterActive> performAnalysis(
          OA_ptr<CallGraph::Interface> callGraph,
          OA_ptr<DataFlow::ParamBindings> paramBind,
          OA_ptr<Alias::InterAliasInterface> interAlias,
          OA_ptr<SideEffect::InterSideEffectInterface> interSE,
          OA_ptr<CFG::EachCFGInterface> eachCFG);

private: // member variables

  OA_ptr<Activity::ActivityIRInterface> mIR;
};

  } // end of Activity namespace
} // end of OA namespace

#endif
