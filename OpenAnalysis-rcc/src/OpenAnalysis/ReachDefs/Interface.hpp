/*! \file
  
  \brief Declarations for the abstract ReachDefs interface.

  \authors Michelle Strout
  \version $Id: Interface.hpp,v 1.6 2004/12/21 21:15:19 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ReachDefsInterface_hpp
#define ReachDefsInterface_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

namespace OA {
  namespace ReachDefs {


class Interface { 
  public:
    Interface() {}
    virtual ~Interface() {}

    typedef StmtHandleIterator ReachDefsIterator;

    //! Return an iterator that will iterate over the set of reaching
    //! defs for the stmt
    virtual OA_ptr<ReachDefsIterator> getReachDefsIterator(StmtHandle s) = 0;

    //! Return an iterator over reaching definitions that reach the exit node
    //! in the procedure
    virtual OA_ptr<Interface::ReachDefsIterator> getExitReachDefsIterator() = 0;

};

  } // end of ReachDefs namespace
} // end of OA namespace

#endif

