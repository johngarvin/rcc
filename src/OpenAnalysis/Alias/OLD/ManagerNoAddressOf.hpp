/*! \file
  
  \brief Declarations of the AnnotationManager that generates an EquivSets

  \authors Michelle Strout
  \version $Id: ManagerNoAddressOf.hpp,v 1.7 2004/11/19 19:21:50 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ALIASMANAGERNOADDRESSOF_H
#define ALIASMANAGERNOADDRESSOF_H

//--------------------------------------------------------------------
#include <map>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Alias/EquivSets.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>

namespace OA {
  namespace Alias {

/*! 
   The AnnotationManager for an EquivSets.  Puts all mem refs into
   the same equiv set except for local vars that do not have their
   address taken.
   This class can build an EquivSets, read one in from a file, and 
   write one out to a file.
*/
class ManagerNoAddressOf { //??? eventually public OA::AnnotationManager
public:
  ManagerNoAddressOf(OA_ptr<AliasIRInterface> _ir) : mIR(_ir) {}
  virtual ~ManagerNoAddressOf () {}

  //! Used to perform analysis when not using AQM
  virtual OA_ptr<Alias::EquivSets> performAnalysis(ProcHandle); 

  //! this method will be used when the AQM is working
  //virtual Alias::EquivSets* performAnalysis(ProcHandle);

private:
  OA_ptr<AliasIRInterface> mIR;

};

  } // end of Alias namespace
} // end of OA namespace

#endif
