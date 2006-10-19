// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
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

// File: LocalityDFSetIterator.cc
//
// Iterator over DFSetElement objects in a DFSet.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/LocalityDFSetElement.h>

#include <cassert>

#include "LocalityDFSetIterator.h"

using namespace OA;

namespace Locality {

DFSetIterator::DFSetIterator (OA_ptr<std::set<OA_ptr<DFSetElement> > > _set)
  : mSet(_set)
{
  assert(!mSet.ptrEqual(NULL));
  mIter = mSet->begin();
}

void DFSetIterator::operator++() {
  if (isValid()) mIter++;
}

//! is the iterator at the end
bool DFSetIterator::isValid() {
  return (mIter != mSet->end());
}

//! return copy of current node in iterator
OA_ptr<DFSetElement> DFSetIterator::current() {
  assert(isValid());
  return (*mIter);
}

//! reset iterator to beginning of set
void DFSetIterator::reset() {
  mIter = mSet->begin();
}

}  // namespace Locality
