/*! \file
  
  \brief Declarations for the abstract interface CFG results by proc

  \authors Michelle Strout
  \version $Id: EachCFGInterface.hpp,v 1.2 2005/06/10 02:32:03 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef EachCFGInterface_H
#define EachCFGInterface_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>

#include <OpenAnalysis/CFG/CFGInterface.hpp>

namespace OA {
  namespace CFG {

class EachCFGInterface {
  public:
    EachCFGInterface() {}
    virtual ~EachCFGInterface() {};

    //! Returns CFG results for the given procedure
    virtual OA_ptr<CFG::CFGInterface> getCFGResults(ProcHandle) = 0;

    //! Returns iterator over procedures with CFG results available
    //FIXME: not really useful because getting CFG on demand
    //virtual OA_ptr<ProcHandleIterator> getKnownProcIterator() = 0;

};

  } // end of CFG namespace
} // end of OA namespace

#endif

