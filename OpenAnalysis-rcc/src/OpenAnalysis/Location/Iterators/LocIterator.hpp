/*! \file
  
  \brief Declarations for the Location iterator class.

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

//! Iterator over locations
class LocIterator {
public:
  LocIterator() { }
  virtual ~LocIterator() { }

  virtual OA_ptr<Location> current() = 0;  // Returns the current item.
  virtual bool isValid() = 0;        // False when all items are exhausted.
        
  virtual void operator++() = 0;
  void operator++(int) { ++*this; } ;

  virtual void reset() = 0;
};

} // end namespace
#endif
