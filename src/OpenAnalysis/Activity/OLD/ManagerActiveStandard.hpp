/*! \file
  
  \brief Declarations of the AnnotationManager that generates a ActiveStandard

  \authors Michelle Strout
  \version $Id: ManagerActiveStandard.hpp,v 1.6 2005/06/10 02:32:02 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ActiveManagerStandard_h
#define ActiveManagerStandard_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include "VaryStandard.hpp"
#include "UsefulStandard.hpp"
#include "ActiveStandard.hpp"

#include <OpenAnalysis/IRInterface/ActivityIRInterface.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
//#include <OpenAnalysis/DataFlow/LocDFSet.hpp>
//#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>

namespace OA {
  namespace Activity {


/*! 
   The AnnotationManager for ActiveStandard.
   This class can build an ActiveStandard, 
   (eventually) read one in from a file, and write one out to a file.
*/

class ManagerActiveStandard {
      //??? eventually public OA::AnnotationManager
public:
  ManagerActiveStandard(OA_ptr<ActivityIRInterface> _ir);
  ~ManagerActiveStandard () {}

  OA_ptr<ActiveStandard> performAnalysis(ProcHandle, 
          OA_ptr<CFG::CFGInterface> cfg,
          OA_ptr<Alias::Interface> alias,
          OA_ptr<VaryStandard> vary,
          OA_ptr<UsefulStandard> useful);
          //OA_ptr<SideEffect::InterSideEffectInterface> interSE,
          //OA_ptr<LocIterator> indepLocIter,
          //OA_ptr<LocIterator> depLocIter);

  /*
  //! USE and MOD sets of side-effect analysis used to seed activity analysis
  OA_ptr<ActiveStandard> performAnalysis(ProcHandle, 
          OA_ptr<CFG::Interface> cfg,
          OA_ptr<Alias::Interface> alias,
          OA_ptr<SideEffect::InterSideEffectInterface> interSE);
          */

private:
// helper function
void calculateActive(
        StmtHandle prevStmt, OA_ptr<LocIterator> prevOutVaryIter,
        StmtHandle stmt, OA_ptr<LocIterator> inUsefulIter);
private:
  OA_ptr<ActivityIRInterface> mIR;
  OA_ptr<ActiveStandard> mActive;
  //OA_ptr<DepStandard> mDep;
  OA_ptr<Alias::Interface> mAlias;

};

  } // end of Activity namespace
} // end of OA namespace

#endif
