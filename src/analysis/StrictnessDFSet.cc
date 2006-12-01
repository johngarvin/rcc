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

// File: StrictnessDFSet.cc
//
// Set of StrictnessDFSetElement objects. Inherits from DataFlowSet for
// use in CFGDFProblem.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/VarRefSet.h>
#include <analysis/LocalityDFSetIterator.h>
#include <analysis/LocalityType.h>

#include "StrictnessDFSet.h"

using namespace OA;

namespace Strictness {

DFSet::DFSet() {
  m_set = new std::set<OA::OA_ptr<DFSetElement> >;
}

DFSet::~DFSet() { }

// After the assignment operation, the lhs DFSet will point to
// the same instances of DFSetElement's that the rhs points to.  Use clone
// if you want separate instances of the DFSetElement's
DFSet& DFSet::operator= (const DFSet& other) {
  m_set = other.m_set;
  return *this;
}

OA_ptr<DataFlow::DataFlowSet> DFSet::clone() {
  OA_ptr<DFSet> retval;
  retval = new DFSet(); 
  std::set<OA_ptr<DFSetElement> >::iterator defIter;
  for (defIter=m_set->begin(); defIter!=m_set->end(); defIter++) {
    OA_ptr<DFSetElement> def = *defIter;
    retval->insert(def);
  }
  return retval;
}
  
void DFSet::insert(OA_ptr<DFSetElement> h) {
  m_set->insert(h); 
}
  
void DFSet::remove(OA_ptr<DFSetElement> h) {
  remove_and_tell(h); 
}

int DFSet::insert_and_tell(OA_ptr<DFSetElement> h) {
  return (int)((m_set->insert(h)).second); 
}

int DFSet::remove_and_tell(OA_ptr<DFSetElement> h) {
  return (m_set->erase(h)); 
}

/// Replace any DFSetElement in m_set with the same location as the given use
void DFSet::replace(OA_ptr<DFSetElement> use) {
  m_set->erase(use);
  m_set->insert(use);
}
  
/// equality: sets are equal if they are the same size and all
/// elements are equal.
bool DFSet::operator==(DataFlow::DataFlowSet &other) const {
  // first dynamic cast to an DFSet, if that doesn't work then 
  // other is a different kind of DataFlowSet and *this is not equal
  DFSet& recastOther = dynamic_cast<DFSet&>(other);

  if (m_set->size() != recastOther.m_set->size()) {
    return false;
  }

  // same size:  for every element in m_set, find the element in other.m_set
  std::set<OA_ptr<DFSetElement> >::iterator set1Iter;
  for (set1Iter = m_set->begin(); set1Iter!=m_set->end(); ++set1Iter) {
    OA_ptr<DFSetElement> cd1 = *set1Iter;
    std::set<OA_ptr<DFSetElement> >::iterator set2Iter;
    set2Iter = recastOther.m_set->find(cd1);

    if (set2Iter == recastOther.m_set->end()) {
      return (false); // cd1 not found
    } else {
      OA_ptr<DFSetElement> cd2 = *set2Iter;
      if (!cd1->equiv(*cd2)) {
        return (false); // cd1 not equiv to cd2
      }
    }
  } // end of set1Iter loop

  // hopefully, if we got here, all elements of set1 equiv set2
  return(true);
}

/// Returns true if there is a VarRef in our set with the same name as
/// the given VarRef. (They don't have to be equivalent VarRefs.)
bool DFSet::includes_name(OA_ptr<R_VarRef> mention) {
  // VarRef's '==' tests if the names are equal, not full equivalence,
  // so 'find' will give us the right answer here.
  return (m_set->find(mention) != m_set->end());
}

/// Set intersection
OA_ptr<DFSet> DFSet::intersect(OA_ptr<DFSet> other) {
  OA_ptr<DFSet> result; result = new DFSet;
  std::set<OA_ptr<R_VarRef> >::iterator it;
  for(it = m_set->begin(); it != m_set->end(); ++it) {
    if (other->member(*it)) {
      result->insert(*it);
    }
  }
  return result;
}

/// Union in a set of variables associated with a given statement
void DFSet::insert_varset(OA_ptr<R_VarRefSet> vars)
{
  OA_ptr<R_VarRefSetIterator> it = vars->get_iterator();
  OA_ptr<DFSetElement> use;
  for (; it->isValid(); ++*it) {
    m_set->insert(it->current());
  }
}


/// Return a string representing the contents of an DFSet
std::string DFSet::toString(OA_ptr<IRHandlesIRInterface> pIR) {
  std::ostringstream oss;
  oss << "{";
  
  // iterate over DFSetElement's and have the IR print them out
  std::set<OA_ptr<R_VarRef> >::iterator iter;
  iter = m_set->begin();

  // first one
  if (iter != m_set->end()) {
    oss << (*iter)->toString(pIR);
    ++iter;
  }
  
  // rest
  for (; iter != m_set->end(); ++iter) {
    oss << ", " << (*iter)->toString(pIR); 
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

}  // namespace Strictness
