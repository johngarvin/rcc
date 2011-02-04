/*! \file
  
  \brief Declarations of the AnnotationManager that generates an InterAliasMap

  \authors Michelle Strout
  \version $Id: ManagerInterAliasMapBasic.hpp,v 1.1 2004/11/30 13:51:33 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef InterMANAGERAliasMapBasic_H
#define InterMANAGERAliasMapBasic_H

//--------------------------------------------------------------------
#include <cassert>

// OpenAnalysis headers
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Alias/InterAliasMap.hpp>
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>

namespace OA {
  namespace Alias {

/*! 
   Will just create a Manager for the intraprocedural results and pass
   that to InterProcedural results, which will then generate intraprocedural
   results on demand.
*/
class ManagerInterAliasMapBasic { //??? eventually public OA::AnnotationManager
public:
  ManagerInterAliasMapBasic(OA_ptr<AliasIRInterface> _ir) : mIR(_ir) {}
  ~ManagerInterAliasMapBasic () {}

  OA_ptr<Alias::InterAliasMap> performAnalysis(OA_ptr<IRProcIterator> procIter);

private:
  OA_ptr<AliasIRInterface> mIR;

};

  } // end of Alias namespace
} // end of OA namespace

#endif
