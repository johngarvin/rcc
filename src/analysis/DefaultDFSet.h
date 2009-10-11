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

// File: DefaultDFSet.h
//
// Set of DefaultDFSetElement objects. Inherits from DataFlowSet for
// use in CFGDFProblem.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef DEFAULT_DF_SET_H
#define DEFAULT_DF_SET_H

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>

#include <analysis/LocalityType.h>

class R_VarRef;
class R_VarRefSet;

typedef R_VarRef DFSetElement;

/// Set of DFSetElement objects. Inherits from DataFlowSet for
/// use in CFGDFProblem.
class DefaultDFSet : public virtual OA::DataFlow::DataFlowSet {
public:

  // construction
  explicit DefaultDFSet();
  ~DefaultDFSet();
  
  /// Create a copy of this set
  OA::OA_ptr<OA::DataFlow::DataFlowSet> clone() const;

  // relationship
  bool operator ==(OA::DataFlow::DataFlowSet &other) const;
  bool operator !=(OA::DataFlow::DataFlowSet &other) const
  { return (!(*this==other)); }

  /// need this one for stl containers
  //  bool operator==(const DefaultDFSet& other) const 
  //  { return DefaultDFSet::operator==(const_cast<DefaultDFSet&>(other)); }

  //************************************************************
  // Modifier Methods
  //************************************************************
  
  /// Set this set to the universal set
  void setUniversal();
  
  /// Remove all elements from this set
  void clear();

  //************************************************************
  // Information Methods
  //************************************************************
  
  /// Return the number of elements contained in this set
  int size() const;
  
  /// Return true if this is the universal set
  bool isUniversalSet() const;
  
  /// Return true if this set is empty
  bool isEmpty() const;
  

  //************************************************************
  // Output and Debugging Methods
  //************************************************************
  void output(OA::IRHandlesIRInterface& ir) const;
  
  /// Output succinct description of set's contents
  void dump(std::ostream &os, OA::OA_ptr<OA::IRHandlesIRInterface> pIR);
  void dump(std::ostream &os);
  
  //------------------------------------------------------------------
  // Our own methods
  //------------------------------------------------------------------

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

  bool member(const OA::OA_ptr<DFSetElement> element) const
  { return (m_set->find(element) != m_set->end()); }

  bool includes_name(OA::OA_ptr<R_VarRef> mention);

  void insert_varset(OA::OA_ptr<R_VarRefSet> vars);

  OA::OA_ptr<DefaultDFSet> intersect(OA::OA_ptr<DefaultDFSet> other);

  OA::OA_ptr<std::set<SEXP> > as_sexp_set();

  // debugging
  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> pIR);
  std::string toString();

private:
  OA::OA_ptr<std::set<OA::OA_ptr<R_VarRef> > > m_set;
};

#endif
