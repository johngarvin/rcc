// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: NameBoolDFSet.h
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>

class NameBoolDFSet : public OA::DataFlow::DataFlowSet {
public:
  class NameBoolPair;

  explicit NameBoolDFSet();
  ~NameBoolDFSet();

  //! Create a copy of this set
  OA::OA_ptr<OA::DataFlow::DataFlowSet> clone() const;
  

  //************************************************************
  // Comparison Operators
  //************************************************************

  /*! Return true if all elements in the LHS are equal to a unique element
    in the RHS and the LHS and RHS have exactly the same number of
    elements. */
  bool operator==(OA::DataFlow::DataFlowSet & other) const;

  //! Return true if the LHS and RHS do not equal each other
  bool operator!=(OA::DataFlow::DataFlowSet & other) const;


  //************************************************************
  // Modifier Methods
  //************************************************************
    
  //! Set this set to the universal set
  void setUniversal();

  //! Remove all elements from this set
  void clear();


  //************************************************************
  // Information Methods
  //************************************************************
    
  //! Return the number of elements contained in this set
  int size() const;

  //! Return true if this is the universal set
  bool isUniversalSet() const;

  //! Return true if this set is empty
  bool isEmpty() const;


  //************************************************************
  // Output and Debugging Methods
  //************************************************************
  void output(OA::IRHandlesIRInterface& ir) const;
    
  //! Output succinct description of set's contents
  void dump(std::ostream &os, OA::OA_ptr<OA::IRHandlesIRInterface>);

  // ----- insert method -----

  void insert(OA::OA_ptr<NameBoolPair>);

  class NameBoolPair {
  public:
    explicit NameBoolPair(SEXP name, bool value);
    bool operator==(const NameBoolPair & other);
    bool operator<(const NameBoolPair & other);
  private:
    SEXP mName;
    bool mValue;
  };

private:
  std::set<OA::OA_ptr<NameBoolPair> > mSet;
};
