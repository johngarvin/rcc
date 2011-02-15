/*! \file
  
  \brief The AnnotationManager that generates InterAliasMaps

  \authors Michelle Strout
  \version $Id: ManagerInterAliasMapBasic.cpp,v 1.1 2004/11/30 13:51:33 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerInterAliasMapBasic.hpp"


namespace OA {
  namespace Alias {

static bool debug = false;

OA_ptr<Alias::InterAliasMap> 
ManagerInterAliasMapBasic::performAnalysis( OA_ptr<IRProcIterator> procIter)
{
  // manager for intraprocedural AliasMap construction
  OA_ptr<ManagerAliasMapBasic> aliasmapman;
  aliasmapman = new ManagerAliasMapBasic(mIR);

  // container for proc to alias results
  OA_ptr<InterAliasMap> retval;
  retval = new InterAliasMap();

  for (procIter->reset();  procIter->isValid(); (*procIter)++ ) {
      ProcHandle proc = procIter->current();

      // store off the aliasmap for this procedure
      retval->mapProcToAliasMap(proc, aliasmapman->performAnalysis(proc));
  }

  return retval;
}

  } // end of namespace Alias
} // end of namespace OA
