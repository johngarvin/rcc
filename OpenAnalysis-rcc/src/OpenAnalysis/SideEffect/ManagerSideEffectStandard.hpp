/*! \file
  
  \brief Declarations of the AnnotationManager that generates a
         SideEffectStandard.

  \authors Michelle Strout, Andy Stone (alias tag update)
  \version $Id: ManagerSideEffectStandard.hpp,v 1.5 2005/08/08 20:03:52 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerSideEffectStandard_h
#define ManagerSideEffectStandard_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/SideEffectIRInterface.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/SideEffect/SideEffectStandard.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>


namespace OA {
  namespace SideEffect {


/*! 
   The AnnotationManager for SideEffectStandard.
   This class can build a SideEffectStandard. 
*/
class ManagerSideEffectStandard {
  //??? eventually public OA::AnnotationManager
public:
  ManagerSideEffectStandard(OA_ptr<SideEffectIRInterface> _ir);
  ~ManagerSideEffectStandard () {}

  //! Used to perform analysis when not using AQM
  OA_ptr<SideEffectStandard> performAnalysis(
    ProcHandle, 
    OA_ptr<Alias::Interface> alias,
    OA_ptr<InterSideEffectInterface> inter);


private: // member variables

  OA_ptr<SideEffectIRInterface> mIR;
  ProcHandle mProc;
};

  } // end of SideEffect namespace
} // end of OA namespace

#endif
