/*! \file
  
  \brief Declarations of the AnnotationManager that generates an EquivSets

  \authors Michelle Strout
  \version $Id: ManagerFIAlias.hpp,v 1.1.2.7 2005/12/09 15:19:44 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ManagerFIAliasEquivSets_H
#define ManagerFIAliasEquivSets_H

//--------------------------------------------------------------------

// OpenAnalysis headers
#include <OpenAnalysis/Alias/EquivSets.hpp>
#include <OpenAnalysis/Alias/ManagerFIAlias.hpp>

namespace OA {
  namespace Alias {

/*! 
    Creates EquivSets for the whole program, which satisfies the 
    Alias::Interface.hpp.  Uses the FIAlias algorithm described
    in the Ryder2001 journal paper and implemented in ManagerFIAlias.
*/
class ManagerFIAliasEquivSets : public ManagerFIAlias 
{ 
public:
  ManagerFIAliasEquivSets(OA_ptr<AliasIRInterface> _ir);
  ~ManagerFIAliasEquivSets () {}

  OA_ptr<Alias::EquivSets> 
  performAnalysis(OA_ptr<IRProcIterator> procIter);

private:

  //! given the union-find datastructure that groups together MREs
  //! create the EquivSets datastructure
  OA_ptr<EquivSets> buildEquivSets( OA_ptr<UnionFindUniverse> ufset,
                                    OA_ptr<IRProcIterator> procIter );
};

  } // end of Alias namespace
} // end of OA namespace

#endif
