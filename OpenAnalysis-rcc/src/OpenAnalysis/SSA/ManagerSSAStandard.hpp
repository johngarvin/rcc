/*! \file
  
  \brief Declarations of the AnnotationManager that generates a SSAStandard.

  \authors Arun Chauhan (2001 as part of Mint), Nathan Tallent, Michelle Strout
  \version $Id: ManagerSSAStandard.hpp,v 1.1 2005/03/08 19:00:12 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef MANAGERSSASTANDARD_H
#define MANAGERSSASTANDARD_H

//--------------------------------------------------------------------
// standard headers
#ifdef NO_STD_CHEADERS
# include <string.h>
#else
# include <cstring>
#endif

// STL headers
#include <list>
#include <set>
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/SSA/SSAStandard.hpp>
#include <OpenAnalysis/IRInterface/SSAIRInterface.hpp>

namespace OA {
  namespace SSA {


//--------------------------------------------------------------------
class Worklist;


//--------------------------------------------------------------------
/*! 
   The AnnotationManager for a SSAStandard Annotation.  Knows how to
   build a SSAStandard, read one in from a file, and write one out to a file.
*/
class ManagerStandard { //??? eventually public OA::AnnotationManager
public:
  ManagerStandard(OA_ptr<SSAIRInterface> _ir) : mIR(_ir) { }
  virtual ~ManagerStandard () { }

  //??? don't think this guy need AQM, but will eventually have
  //to have one so is standard with other AnnotationManagers
  virtual OA_ptr<SSA::SSAStandard> performAnalysis(ProcHandle,
						   OA_ptr<CFG::CFGInterface> cfg);

  //-------------------------------------
  // information access
  //-------------------------------------
  OA_ptr<SSAIRInterface> getIRInterface() { return mIR; }

  //------------------------------------------------------------------
  // Exceptions
  //------------------------------------------------------------------
  class SSAException : public Exception {
  public:
    void report(std::ostream& os) const 
      { os << "E!  Unexpected." << std::endl; }
  };

private:
 
  //------------------------------------------------------------------
  // Methods that build SSAStandard
  //------------------------------------------------------------------
  void build(OA_ptr<CFG::CFGInterface> cfg);

private:
  OA_ptr<SSAIRInterface> mIR;
  OA_ptr<SSAStandard> mSSA;
};
//--------------------------------------------------------------------

  } // end of SSA namespace
} // end of OA namespace

#endif
