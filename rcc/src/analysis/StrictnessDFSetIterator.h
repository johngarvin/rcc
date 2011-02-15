// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: StrictnessDFSetIterator.h
//
// Iterator over DFSetElement objects in a DFSet.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef STRICTNESS_DF_SET_ITERATOR_H
#define STRICTNESS_DF_SET_ITERATOR_H

#include <set>

#include <OpenAnalysis/Utils/OA_ptr.hpp>

#include <analysis/StrictnessDFSetElement.h>

namespace Strictness {

/// Iterator over each DFSetElement in an DFSet
class DFSetIterator {
public:
  explicit DFSetIterator (OA::OA_ptr<std::set<OA::OA_ptr<DFSetElement> > > _set);
  ~DFSetIterator () {}
  
  void operator++();
  bool isValid() const;
  OA::OA_ptr<DFSetElement> current() const;
  void reset();

private:
  OA::OA_ptr<std::set<OA::OA_ptr<DFSetElement> > > mSet;
  std::set<OA::OA_ptr<DFSetElement> >::const_iterator mIter;
};

}  // namespace Strictness

#endif
