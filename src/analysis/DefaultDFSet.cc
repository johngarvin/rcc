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

// File: DefaultDFSet.cc
//
// Set of DFSetElement objects. Inherits from DataFlowSet for use in
// CFGDFProblem.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/VarRefSet.h>
#include <analysis/LocalityDFSetIterator.h>
#include <analysis/LocalityType.h>

#include "DefaultDFSet.h"

using namespace OA;

DefaultDFSet::DefaultDFSet() {
  m_set = new std::set<OA::OA_ptr<DFSetElement> >;
}

DefaultDFSet::~DefaultDFSet() { }

// After the assignment operation, the lhs DFSet will point to
// the same instances of DFSetElement's that the rhs points to.  Use clone
// if you want separate instances of the DFSetElement's
DefaultDFSet& DefaultDFSet::operator= (const DefaultDFSet& other) {
  m_set = other.m_set;
  return *this;
}

OA_ptr<DataFlow::DataFlowSet> DefaultDFSet::clone() {
  OA_ptr<DefaultDFSet> retval;
  retval = new DefaultDFSet(); 
  std::set<OA_ptr<DFSetElement> >::const_iterator defIter;
  for (defIter=m_set->begin(); defIter!=m_set->end(); defIter++) {
    OA_ptr<DFSetElement> def = *defIter;
    retval->insert(def);
  }
  return retval;
}
  
void DefaultDFSet::insert(OA_ptr<DFSetElement> h) {
  m_set->insert(h); 
}
  
void DefaultDFSet::remove(OA_ptr<DFSetElement> h) {
  remove_and_tell(h); 
}

int DefaultDFSet::insert_and_tell(OA_ptr<DFSetElement> h) {
  return (int)((m_set->insert(h)).second); 
}

int DefaultDFSet::remove_and_tell(OA_ptr<DFSetElement> h) {
  return (m_set->erase(h)); 
}

/// Replace any DFSetElement in m_set with the same location as the given use
void DefaultDFSet::replace(OA_ptr<DFSetElement> use) {
  m_set->erase(use);
  m_set->insert(use);
}
  
/// equality: sets are equal if they are the same size and all
/// elements are equal.
bool DefaultDFSet::operator==(DataFlow::DataFlowSet &other) const {
  // first dynamic cast to an DefaultDFSet, if that doesn't work then 
  // other is a different kind of DataFlowSet and *this is not equal
  DefaultDFSet& recastOther = dynamic_cast<DefaultDFSet&>(other);

  if (m_set->size() != recastOther.m_set->size()) {
    return false;
  }

  // same size:  for every element in m_set, find the element in other.m_set
  std::set<OA_ptr<DFSetElement> >::const_iterator set1Iter;
  for (set1Iter = m_set->begin(); set1Iter!=m_set->end(); ++set1Iter) {
    OA_ptr<DFSetElement> cd1 = *set1Iter;
    std::set<OA_ptr<DFSetElement> >::const_iterator set2Iter;
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

/// Returns true if there is a VarRef in our set with the given name.
bool DefaultDFSet::includes_name(OA_ptr<R_VarRef> mention) {
  // VarRef's '==' tests if the names are equal, not full equivalence,
  // so 'find' will give us the right answer here.
  return (m_set->find(mention) != m_set->end());
}

/// Set intersection
OA_ptr<DefaultDFSet> DefaultDFSet::intersect(OA_ptr<DefaultDFSet> other) {
  OA_ptr<DefaultDFSet> result; result = new DefaultDFSet;
  std::set<OA_ptr<R_VarRef> >::const_iterator it;
  for (it = m_set->begin(); it != m_set->end(); it++) {
    if (other->member(*it)) {
      result->insert(*it);
    }
  }
  return result;
}

/// Union in a set of variables associated with a given statement
void DefaultDFSet::insert_varset(OA_ptr<R_VarRefSet> vars)
{
  OA_ptr<R_VarRefSetIterator> it = vars->get_iterator();
  OA_ptr<DFSetElement> use;
  for (; it->isValid(); ++*it) {
    m_set->insert(it->current());
  }
}

OA_ptr<std::set<SEXP> > DefaultDFSet::as_sexp_set() {
  OA_ptr<std::set<SEXP> > retval; retval = new std::set<SEXP>;
  std::set<OA_ptr<R_VarRef> >::const_iterator it;
  for (it = m_set->begin(); it != m_set->end(); it++) {
    retval->insert((*it)->get_sexp());
  }
  return retval;
}

/// Return a string representing the contents of an DefaultDFSet
std::string DefaultDFSet::toString() {
  std::ostringstream oss;
  oss << "{";
  
  // iterate over DFSetElement's and have the IR print them out
  std::set<OA_ptr<R_VarRef> >::const_iterator iter;
  iter = m_set->begin();

  // first one
  if (iter != m_set->end()) {
    oss << (*iter)->toString();
    ++iter;
  }
  
  // rest
  for (; iter != m_set->end(); ++iter) {
    oss << ", " << (*iter)->toString(); 
  }
  
  oss << "}";
  return oss.str();
}

void DefaultDFSet::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> pIR) {
  dump(os);
}

void DefaultDFSet::dump(std::ostream &os) {
  os << toString() << std::endl;
}

