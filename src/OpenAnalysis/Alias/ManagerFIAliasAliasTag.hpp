/*! \file
  
  \brief Declarations of the AnnotationManager that generates AliasTag
         results from FIAlias.

  \authors Michelle Strout, Brian White

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerFIAliasAliasTag_H
#define ManagerFIAliasAliasTag_H

//--------------------------------------------------------------------

// OpenAnalysis headers
#include <OpenAnalysis/Alias/ManagerFIAlias.hpp>
#include <OpenAnalysis/Alias/AliasTagResults.hpp>
#include <map>

namespace OA {
  namespace Alias {

/*! 
*/
class ManagerFIAliasAliasTag : public ManagerFIAlias 
{ 
public:
  ManagerFIAliasAliasTag(OA_ptr<AliasIRInterface> _ir);
  ~ManagerFIAliasAliasTag () {}

  //! Perform the inter-procedural FIAlias analysis.
  OA_ptr<Alias::Interface> 
  performAnalysis(OA_ptr<IRProcIterator> procIter,
                  FIAliasImplement implement = ALL_PROCS );

private:

  //! given the union-find datastructure that groups together MREs
  //! create the AliasTagResults datastructure
  void buildAliasMaps( OA_ptr<UnionFindUniverse> ufset );

};

  } // end of Alias namespace
} // end of OA namespace

#endif
