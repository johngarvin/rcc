/*! \file
  
NOTE: deprecated!!  No longer being compiled.
6/2/06, Removed ManagerInsNoPtrInterAliasMap because it is no longer
valid.  It used the CallGraph data-flow analysis framework
and dependended on the isRefParam() call, which has been deprecated.

  \brief Declarations of the AnnotationManager that generates an
         SymAliasSet that is optimistic, each non-local and reference
         param is in its own set.

  \authors Michelle Strout
  \version $Id: ManagerSymAliasSetsTop.hpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerSymAliasSets_H
#define ManagerSymAliasSets_H

//--------------------------------------------------------------------

// OpenAnalysis headers
#include "SymAliasSets.hpp"
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>

namespace OA {
  namespace Alias {

/*! 
   The AnnotationManager for an SymAliasSets. Just create one, 
   the default is that each symbol maps to itself.
*/
class ManagerSymAliasSetsTop {
public:
  ManagerSymAliasSetsTop(OA_ptr<AliasIRInterface> _ir) : mIR(_ir) {}
  ~ManagerSymAliasSetsTop () {}

  OA_ptr<Alias::SymAliasSets> performAnalysis(ProcHandle proc); 

private:
  OA_ptr<AliasIRInterface> mIR;
  ProcHandle mProc;

};

  } // end of Alias namespace
} // end of OA namespace

#endif
