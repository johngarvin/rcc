/*! \file
  
NOTE: deprecated!!  No longer being compiled.
6/2/06, Removed ManagerInsNoPtrInterAliasMap because it is no longer
valid.  It used the CallGraph data-flow analysis framework
and dependended on the isRefParam() call, which has been deprecated.

  \brief Declarations of the AnnotationManager that generates an
         SymAliasSets that is pessimistic, all non-local and reference
         param symbols are in the same set.

  \authors Michelle Strout
  \version $Id: ManagerSymAliasSetsBottom.hpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerSymAliasSetsBottom_H
#define ManagerSymAliasSetsBottom_H

//--------------------------------------------------------------------

// OpenAnalysis headers
#include "SymAliasSets.hpp"
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExprVisitor.hpp>

namespace OA {
  namespace Alias {

/*! 
   The AnnotationManager for an SymAliasSets. Just create one, 
   the default is that each symbol maps to itself.
*/
class ManagerSymAliasSetsBottom {
public:
  ManagerSymAliasSetsBottom(OA_ptr<AliasIRInterface> _ir) : mIR(_ir) {}
  ~ManagerSymAliasSetsBottom () {}

  OA_ptr<Alias::SymAliasSets> performAnalysis(ProcHandle); 

private:
  OA_ptr<AliasIRInterface> mIR;
  ProcHandle mProc;

};

  } // end of Alias namespace
} // end of OA namespace

#endif
