/*! \file
  
NOTE: deprecated!!  No longer being compiled.
6/2/06, Removed ManagerInsNoPtrInterAliasMap because it is no longer
valid.  It used the CallGraph data-flow analysis framework
and dependended on the isRefParam() call, which has been deprecated.

  \brief The AnnotationManager that generates SymAliasSets top

  \authors Michelle Strout
  \version $Id: ManagerSymAliasSetsTop.cpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerSymAliasSetsTop.hpp"
#include <Utils/Util.hpp>


namespace OA {
  namespace Alias {

static bool debug = false;

/*!
   Just creates default because default maps each symbol to self.
   In other words, it is assumed that no globals or reference parameters
   may alias each other.
*/
OA_ptr<Alias::SymAliasSets> 
ManagerSymAliasSetsTop::performAnalysis(ProcHandle proc) 
{
  OA_DEBUG_CTRL_MACRO("DEBUG_ManagerSymAliasSetsTop:ALL", debug);
  OA_ptr<SymAliasSets> retMap; retMap = new Alias::SymAliasSets();
  return retMap;
}

  } // end of namespace Alias
} // end of namespace OA
