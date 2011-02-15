/*! \file
  
  \brief Declarations of the AnnotationManager that generates AliasMaps.

  \authors Michelle Strout, Brian White
  \version $Id: ManagerFIAlias.hpp,v 1.1.2.7 2005/12/09 15:19:44 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerFIAliasAliasMap_H
#define ManagerFIAliasAliasMap_H

//--------------------------------------------------------------------

// OpenAnalysis headers
#include <OpenAnalysis/Alias/InterAliasMap.hpp>
#include <OpenAnalysis/Alias/ManagerFIAlias.hpp>
#include <map>

namespace OA {
  namespace Alias {

/*! 
    Creates AliasMaps for the whole program, 
    which satisfie the Alias::Interface.hpp.  
    Uses the FIAlias algorithm described
    in the Ryder2001 journal paper and implemented in ManagerFIAlias.
*/
class ManagerFIAliasAliasMap : public ManagerFIAlias 
{ 
public:
  ManagerFIAliasAliasMap(OA_ptr<AliasIRInterface> _ir);
  ~ManagerFIAliasAliasMap () {}

  //! Perform the inter-procedural FIAlias analysis.
  OA_ptr<Alias::InterAliasMap> 
  performAnalysis(OA_ptr<IRProcIterator> procIter,
                  FIAliasImplement implement = ALL_PROCS );

private:

  //! given the union-find datastructure that groups together MREs
  //! create the AliasMap datastructures
  void buildAliasMaps( OA_ptr<UnionFindUniverse> ufset,
                       OA_ptr<IRProcIterator> procIter );

  //! A map from procedures to alias maps.
  OA_ptr<InterAliasMap> mInterAliasMap;

};

  } // end of Alias namespace
} // end of OA namespace

#endif
