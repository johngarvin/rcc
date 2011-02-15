/*! \file
  
  \authors Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LivenessInterface_hpp
#define LivenessInterface_hpp

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Alias/AliasTag.hpp>

namespace OA {
  namespace Liveness {

class LivenessIterator {
  public:
    LivenessIterator() { }
    virtual ~LivenessIterator() { };

    virtual Alias::AliasTag current() const = 0;
    virtual bool isValid() const = 0;

    virtual void operator++() = 0;
    void operator++(int) { ++*this; }

    virtual void reset() = 0;
};


class Interface { 
  public:
    Interface() {}
    virtual ~Interface() {}

    //! Return an iterator that will iterate over the set of alias tags
    //! live for the stmt
    virtual OA_ptr<LivenessIterator> getLivenessIterator(StmtHandle s) const
        = 0;

    //! Return an iterator over tags that are live out of the the exit node
    //! in the procedure
    virtual OA_ptr<LivenessIterator> getExitLivenessIterator() const = 0;

};

  } // end of Liveness namespace
} // end of OA namespace

#endif


