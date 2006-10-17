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

// File: LocalityDFSet.cc
//
// Set of LocalityDFSetElement objects. Inherits from DataFlowSet for
// use in CFGDFProblem.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/VarRefSet.h>
#include <analysis/LocalityDFSetElement.h>
#include <analysis/LocalityDFSetIterator.h>
#include <analysis/LocalityType.h>

#include "LocalityDFSet.h"

using namespace OA;

namespace Locality {

DFSet::DFSet() {
  mSet = new std::set<OA::OA_ptr<DFSetElement> >;
}

DFSet::DFSet(const DFSet& other) : mSet(other.mSet) { }

DFSet::~DFSet() { }

// After the assignment operation, the lhs DFSet will point to
// the same instances of DFSetElement's that the rhs points to.  Use clone
// if you want separate instances of the DFSetElement's
DFSet& DFSet::operator= (const DFSet& other) {
  mSet = other.mSet; 
  return *this;
}

OA_ptr<DataFlow::DataFlowSet> DFSet::clone() {
  OA_ptr<DFSet> retval;
  retval = new DFSet(); 
  std::set<OA_ptr<DFSetElement> >::iterator defIter;
  for (defIter=mSet->begin(); defIter!=mSet->end(); defIter++) {
    OA_ptr<DFSetElement> def = *defIter;
    retval->insert(def->clone());
  }
  return retval;
}
  
void DFSet::insert(OA_ptr<DFSetElement> h) {
  mSet->insert(h); 
}
  
void DFSet::remove(OA_ptr<DFSetElement> h) {
  remove_and_tell(h); 
}

int DFSet::insert_and_tell(OA_ptr<DFSetElement> h) {
  return (int)((mSet->insert(h)).second); 
}

int DFSet::remove_and_tell(OA_ptr<DFSetElement> h) {
  return (mSet->erase(h)); 
}

//! Replace any DFSetElement in mSet with the same location as the given use
void DFSet::replace(OA_ptr<DFSetElement> use) {
    replace(use->get_loc(), use->get_locality_type());
}

//! replace any DFSetElement in mSet with location loc 
//! with DFSetElement(loc,type)
void DFSet::replace(OA_ptr<R_VarRef> loc, LocalityType type) {
  OA_ptr<DFSetElement> use;
  use = new DFSetElement(loc, Locality_TOP);
  remove(use);
  use = new DFSetElement(loc,type);
  insert(use);
}

//! operator== for an DFSet cannot rely upon the == operator for
//! the underlying sets, because the == operator of an element of a
//! DFSet, namely an DFSetElement, only considers the contents of the
//! location pointer and not any of the other fields.  So, need to use
//! DFSetElement's equal() method here instead.
bool DFSet::operator==(DataFlow::DataFlowSet &other) const {
  // first dynamic cast to an DFSet, if that doesn't work then 
  // other is a different kind of DataFlowSet and *this is not equal
  DFSet& recastOther = dynamic_cast<DFSet&>(other);

  if (mSet->size() != recastOther.mSet->size()) {
    return false;
  }

  // same size:  for every element in mSet, find the element in other.mSet
  std::set<OA_ptr<DFSetElement> >::iterator set1Iter;
  for (set1Iter = mSet->begin(); set1Iter!=mSet->end(); ++set1Iter) {
    OA_ptr<DFSetElement> cd1 = *set1Iter;
    std::set<OA_ptr<DFSetElement> >::iterator set2Iter;
    set2Iter = recastOther.mSet->find(cd1);

    if (set2Iter == recastOther.mSet->end()) {
      return (false); // cd1 not found
    } else {
      OA_ptr<DFSetElement> cd2 = *set2Iter;
      if (!(cd1->equiv(*cd2))) { // use full equiv operator
        return (false); // cd1 not equiv to cd2
      }
    }
  } // end of set1Iter loop

  // hopefully, if we got here, all elements of set1 equiv set2
  return(true);
}

//! find the DFSetElement in this DFSet with the given location (should
//! be at most one) return a ptr to that DFSetElement
OA_ptr<DFSetElement> DFSet::find(OA_ptr<R_VarRef> loc) const {
  OA_ptr<DFSetElement> retval; retval = NULL;
  
  OA_ptr<DFSetElement> find_var; find_var = new DFSetElement(loc, Locality_TOP);

  std::set<OA_ptr<DFSetElement> >::iterator iter = mSet->find(find_var);
  if (iter != mSet->end()) {
    retval = *iter;
  }
  return retval;
}

//! Union in a set of variables associated with a given statement
void DFSet::insert_varset(OA_ptr<R_VarRefSet> vars,
			    LocalityType type)
{
  OA_ptr<R_VarRefSetIterator> it = vars->get_iterator();
  OA_ptr<DFSetElement> use;
  for (; it->isValid(); ++*it) {
    replace(it->current(),type);
  }
}


//! Return a string representing the contents of an DFSet
std::string DFSet::toString(OA_ptr<IRHandlesIRInterface> pIR) {
  std::ostringstream oss;
  oss << "{";
  
  // iterate over DFSetElement's and have the IR print them out
  OA_ptr<DFSetIterator> iter = get_iterator();
  OA_ptr<DFSetElement> use;
  
  // first one
  if (iter->isValid()) {
    use = iter->current();
    oss << use->toString(pIR);
    ++*iter;
  }
  
  // rest
  for (; iter->isValid(); ++*iter) {
    use = iter->current();
    oss << ", " << use->toString(pIR); 
  }
  
  oss << "}";
  return oss.str();
}

void DFSet::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> pIR) {
  os << toString(pIR) << std::endl;
}

void DFSet::dump(std::ostream &os) {
  std::cout << "call dump(os,interface) instead";
}

OA_ptr<DFSetIterator> DFSet::get_iterator() const {
  OA_ptr<DFSetIterator> it;
  it = new DFSetIterator(mSet);
  return it;
}

}  // namespace Locality
