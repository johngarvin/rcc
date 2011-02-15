/*! \file
  
  \brief Declarations of the AnnotationManager that generates an AliasMap
         for a given procedure.

  \authors Michelle Strout
  \version $Id: ManagerAliasMapBasic.hpp,v 1.9.6.1 2006/01/18 23:28:42 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ALIASMANAGERAliasMapBasic_H
#define ALIASMANAGERAliasMapBasic_H

//--------------------------------------------------------------------
#include <cassert>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Alias/AliasMap.hpp>

#include <OpenAnalysis/MemRefExpr/MemRefExprVisitor.hpp>
#include <OpenAnalysis/Location/Locations.hpp>
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>

namespace OA {
  namespace Alias {

/*! 
   The AnnotationManager for an AliasMap.  Local var refs that have no
   overlapping location blocks (due to equivalences or unions for
   example) are mapped to their own entry in the AliasMap with only
   one possible location.  All other memory references are mapped
   to the zeroth AliasMap entry which indicates that the locations
   they may reference are unknown.
   This class can build an AliasMap, (eventually) read one in from a file, 
   and write one out to a file.
*/
class ManagerAliasMapBasic { //??? eventually public OA::AnnotationManager
public:
  ManagerAliasMapBasic(OA_ptr<AliasIRInterface> _ir) : mIR(_ir) {}
  ~ManagerAliasMapBasic () {}

  OA_ptr<Alias::AliasMap> performAnalysis(ProcHandle proc);

  /*
  OA_ptr<Alias::AliasMap> performAnalysis(ProcHandle proc) 
  {
    // optimistic assumption that reference parameters don't alias
    // each other or globals
    OA_ptr<AliasMap> seedAliasMap;
    seedAliasMap = new AliasMap(proc);
    return performAnalysis(proc,seedAliasMap);
  }
  */

  //! do a trivial map of mre to loc if possible (no addressTaken
  //! and no dereferencing), otherwise return NULL
  //OA_ptr<Location> trivialMREToLoc(OA_ptr<MemRefExpr> mre);

private:
  OA_ptr<AliasIRInterface> mIR;
  ProcHandle mProc;

};

  } // end of Alias namespace
} // end of OA namespace

#endif
