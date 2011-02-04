/*! \file
  
  \brief Declarations of the AnnotationManager that generates Activity
         analysis results for each procedure intraprocedurally

  \authors Michelle Strout
  \version $Id: ManagerEachActive.hpp,v 1.2 2005/02/28 20:21:28 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerEachActive_h
#define ManagerEachActive_h

//--------------------------------------------------------------------
// OpenAnalysis headers

#include <OpenAnalysis/Activity/ManagerActiveStandard.hpp>
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
class ManagerEachActive {
public:
  ManagerEachActive(OA_ptr<Activity::ActivityIRInterface> _ir);
  ~ManagerEachActive () {}

  OA_ptr<InterActive> performAnalysis(
          OA_ptr<IRProcIterator> procIter,
          OA_ptr<Alias::InterAliasInterface> interAlias,
          OA_ptr<SideEffect::InterSideEffectInterface> interSE,
          OA_ptr<CFG::EachCFGInterface> eachCFG);

private: // member variables

  OA_ptr<Activity::ActivityIRInterface> mIR;
};

  } // end of Activity namespace
} // end of OA namespace

#endif
