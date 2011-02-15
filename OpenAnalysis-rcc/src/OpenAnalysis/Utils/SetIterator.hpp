/*! \file
  
  \brief Templated implementation of an OpenAnalysis iterator interface
         for a set of objects.

  \authors Michelle Strout
  \version $Id: SetIterator.hpp,v 1.2 2004/11/19 19:21:53 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef SetIterator_H
#define SetIterator_H

// STL headers
#include <set>

#include <OpenAnalysis/Utils/OA_ptr.hpp>

namespace OA {

template <class T>
class SetIterator {
public:
    SetIterator (OA_ptr<std::set<T> > pSet) : mSet(pSet), mIter(pSet->begin()) 
    {}
    virtual ~SetIterator () {}

    void operator++() { if (mIter != mSet->end()) { mIter++; } }
    void operator++(int) { ++*this; } 

    //! returns true if we are not past the end of the set
    bool isValid() { return (mIter != mSet->end()); }

    //! returns the statement handle that the iterator is currently pointing 
    T current() { return *mIter; }

    void reset() { mIter = mSet->begin(); }

  private:
    OA_ptr<std::set<T> > mSet;
    typename std::set<T>::iterator mIter;
};

} // end of OA namespace

#endif
