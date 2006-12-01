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

// File: StrictnessDFSet.h
//
// Set of StrictnessDFSetElement objects. Inherits from DataFlowSet for
// use in CFGDFProblem.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef STRICTNESS_DF_SET_H
#define STRICTNESS_DF_SET_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/LocalityType.h>

class R_VarRef;
class R_VarRefSet;

typedef R_VarRef DFSetElement;

namespace Strictness {

/// Set of StrictnessDFSetElement objects. Inherits from DataFlowSet for
/// use in CFGDFProblem.
// Removed "virtual": need clone() to be able to return an DFSet.
//class DFSet : public virtual OA::DataFlow::DataFlowSet {
class DFSet : public OA::DataFlow::DataFlowSet {
public:

  // construction
  DFSet();
  ~DFSet();
  
  // After the assignment operation, the lhs DFSet will point to the
  // same instances of DFSetElements that the rhs points to.  Use
  // clone if you want separate instances of the DFSetElements.
  DFSet& operator= (const DFSet& other);
  OA::OA_ptr<OA::DataFlow::DataFlowSet> clone();
  
  void insert(OA::OA_ptr<DFSetElement> h);
  void remove(OA::OA_ptr<DFSetElement> h);
  int insert_and_tell(OA::OA_ptr<DFSetElement> h);
  int remove_and_tell(OA::OA_ptr<DFSetElement> h);

  /// replace any DFSetElement in m_set with location locPtr 
  /// with DFSetElement(locPtr,cdType)
  /// must use this instead of insert because std::set::insert will just see
  /// that the element with the same locptr is already in the set and then not
  /// insert the new element
  void replace(OA::OA_ptr<DFSetElement> ru);

  // relationship
  // these implement virtual methods from DataFlowSet.
  bool operator ==(OA::DataFlow::DataFlowSet &other) const;
  bool operator !=(OA::DataFlow::DataFlowSet &other) const
  { return (!(*this==other)); }

  /// need this one for stl containers
  bool operator==(const DFSet& other) const 
  { return DFSet::operator==(const_cast<DFSet&>(other)); }

  bool member(const OA::OA_ptr<DFSetElement> element) const
  { return (m_set->find(element) != m_set->end()); }

  bool includes_name(OA::OA_ptr<R_VarRef> mention);

  bool empty() const { return m_set->empty(); }
  
  void insert_varset(OA::OA_ptr<R_VarRefSet> vars);

  OA::OA_ptr<DFSet> intersect(OA::OA_ptr<DFSet> other);

  // debugging
  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> pIR);
  std::string toString();
  void dump(std::ostream &os, OA::OA_ptr<OA::IRHandlesIRInterface> pIR);
  void dump(std::ostream &os);

private:
  OA::OA_ptr<std::set<OA::OA_ptr<R_VarRef> > > m_set;
};

}  // namespace Strictness

#endif
