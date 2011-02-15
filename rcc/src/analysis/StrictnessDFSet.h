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

// File: StrictnessDFSet.h
//
// Set of StrictnessDFSetElement objects. Inherits from DataFlowSet for
// use in CFGDFProblem.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef STRICTNESS_DF_SET_H
#define STRICTNESS_DF_SET_H

#include <set>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>

#include <analysis/StrictnessType.h>

class R_VarRef;
class R_VarRefSet;

namespace Strictness {

class DFSetElement;
class DFSetIterator;

/// Set of StrictnessDFSetElement objects. Inherits from DataFlowSet for
/// use in CFGDFProblem.
class DFSet : public virtual OA::DataFlow::DataFlowSet {
private:
  // typedef for readability
  typedef std::set<OA::OA_ptr<DFSetElement> > MySetT;

public:

  // construction
  explicit DFSet();
  explicit DFSet(const DFSet & other);
  ~DFSet();
  
  // ----- methods inherited from DataFlowSet -----

  OA::OA_ptr<OA::DataFlow::DataFlowSet> clone() const;
  
  // relationship
  // param for these can't be const because will have to 
  // dynamic cast to specific subclass
  bool operator ==(OA::DataFlow::DataFlowSet & other) const;
  bool operator !=(OA::DataFlow::DataFlowSet & other) const
  { return (!(*this==other)); }

  /// Set this set to the universal set
  void setUniversal();

  /// Remove all elements from this set
  void clear();

  /// Return the number of elements contained in this set
  int size() const;

  /// Return true if this is the universal set
  bool isUniversalSet() const;

  /// Return true if this set is empty
  bool isEmpty() const;
  
  // debugging
  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> pIR);
  std::string toString();
  void output(OA::IRHandlesIRInterface & ir) const;
  void dump(std::ostream & os, OA::OA_ptr<OA::IRHandlesIRInterface> pIR);
  void dump(std::ostream & os);

  // ----- our own methods -----

  void insert(OA::OA_ptr<DFSetElement> h);
  void remove(OA::OA_ptr<DFSetElement> h);
  int insert_and_tell(OA::OA_ptr<DFSetElement> h);
  int remove_and_tell(OA::OA_ptr<DFSetElement> h);

  /// replace any DFSetElement in mSet with location locPtr 
  /// with DFSetElement(locPtr,cdType)
  /// must use this instead of insert because std::set::insert will just see
  /// that the element with the same locptr is already in the set and then not
  /// insert the new element
  void replace(OA::OA_ptr<R_VarRef> loc, StrictnessType type);
  void replace(OA::OA_ptr<DFSetElement> ru);

  /// Return pointer to a copy of a DFSetElement in this set with matching loc
  /// NULL is returned if no DFSetElement in this set matches loc
  OA::OA_ptr<DFSetElement> find(OA::OA_ptr<R_VarRef> locPtr) const;

  void insert_varset(OA::OA_ptr<R_VarRefSet> vars, StrictnessType type);

  bool includes_name(OA::OA_ptr<R_VarRef> mention);

  OA::OA_ptr<DFSetIterator> get_iterator() const;
  
protected:
  OA::OA_ptr<MySetT> mSet;

  friend class DFSetIterator;
};

}  // namespace Strictness

#endif
