/*! \file
  
  \brief Declarations of the AnnotationManager that generates a 
         UDDUChainsStandard. 

  \authors Michelle Strout
  \version $Id: ManagerUDDUChainsStandard.hpp,v 1.6 2005/03/18 18:14:16 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerUDDUChainsStandard_h
#define ManagerUDDUChainsStandard_h

//--------------------------------------------------------------------
// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/UDDUChainsIRInterface.hpp>
#include <OpenAnalysis/ReachDefs/Interface.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/UDDUChains/UDDUChainsStandard.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectInterface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExprVisitor.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/ExprTree/CollectMREVisitor.hpp>

namespace OA {
  namespace UDDUChains {


/*! 
   The AnnotationManager for UDDUChainsStandard.
   This class can build an UDDUChainsStandard, 
   (eventually) read one in from a file, and write one out to a file.
*/
class ManagerUDDUChainsStandard {
      //??? eventually public OA::AnnotationManager
public:
  ManagerUDDUChainsStandard(OA_ptr<UDDUChainsIRInterface> _ir);
  ~ManagerUDDUChainsStandard () {}
  OA_ptr<UDDUChainsStandard> performAnalysis(ProcHandle, 
        OA_ptr<Alias::Interface> alias, OA_ptr<ReachDefs::Interface> reachDefs,
        OA_ptr<SideEffect::InterSideEffectInterface> interSE);


private:
  OA_ptr<UDDUChainsIRInterface> mIR;


};


  } // end of UDDUChains namespace
} // end of OA namespace

#endif
