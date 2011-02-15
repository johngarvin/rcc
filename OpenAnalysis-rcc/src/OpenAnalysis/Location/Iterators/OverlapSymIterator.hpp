/*! \file
  
  \brief Declaration for the overlapping symbol iterator class.

  \authors Michelle Strout, Andy Stone
  \version $Id: Location.hpp,v 1.26.6.1 2005/11/04 16:24:12 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef LocIterator_H
#define LocIterator_H

#include <OpenAnalysis/Location/Location.hpp>

namespace OA {

//! An iterator over Symbols
class OverlapSymIterator : public virtual OA::SymHandleIterator,
                           public OA::IRHandleSetIterator<OA::SymHandle>
{
public:
  OverlapSymIterator(OA::OA_ptr<set<OA::SymHandle> > pSet) 
    : OA::IRHandleSetIterator<OA::SymHandle>(pSet) {}
  ~OverlapSymIterator() {}

  void operator++() { OA::IRHandleSetIterator<OA::SymHandle>::operator++(); }
  bool isValid() const
    { return OA::IRHandleSetIterator<OA::SymHandle>::isValid(); }
  OA::SymHandle current() const
    { return OA::IRHandleSetIterator<OA::SymHandle>::current(); }
  void reset() { OA::IRHandleSetIterator<OA::SymHandle>::current(); }
};

} // end of OA namespace
#endif
