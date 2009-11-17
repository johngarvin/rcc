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

//#include <include/R/R_RInternals.h>

#ifndef NAME_BOOL_DF_SET_H
#define NAME_BOOL_DF_SET_H

#include <OpenAnalysis/DataFlow/DataFlowSet.hpp>

#include <analysis/VarRef.h>

class NameBoolDFSet : public OA::DataFlow::DataFlowSet {
public:
  class NameBoolPair;
  class NameBoolDFSetIterator;

  typedef std::set<OA::OA_ptr<NameBoolPair> > MySet;

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
  void dump(std::ostream & os, OA::OA_ptr<OA::IRHandlesIRInterface>);

  void dump(std::ostream & os);

  void dump();

  // ----- our own methods -----

  void insert(OA::OA_ptr<NameBoolPair>);

  OA::OA_ptr<NameBoolDFSet> meet(OA::OA_ptr<NameBoolDFSet> other);

  OA::OA_ptr<NameBoolDFSetIterator> getIterator() const;

  bool lookup(OA::OA_ptr<R_VarRef> e) const;

  void replace(OA::OA_ptr<R_VarRef> e, bool value);

  typedef bool (* PredicateType)(SEXP call, int arg);
  static void propagate_lhs(OA::OA_ptr<NameBoolDFSet> in, PredicateType predicate, bool value, SEXP cell);
  static void propagate_rhs(OA::OA_ptr<NameBoolDFSet> in, PredicateType predicate, bool value, SEXP cell);

  class NameBoolPair {
  public:
    explicit NameBoolPair(OA::OA_ptr<R_VarRef> name, bool value);
    bool operator==(const NameBoolPair & other);
    bool equiv(const NameBoolPair & other);
    bool operator<(const NameBoolPair & other);
    OA::OA_ptr<R_VarRef> getName();
    bool getValue();
    void setValue(bool x);
  private:
    OA::OA_ptr<R_VarRef> mName;
    bool mValue;
  };

  class NameBoolDFSetIterator {
  public:
    explicit NameBoolDFSetIterator(OA::OA_ptr<MySet> set);
    ~NameBoolDFSetIterator();

    void operator++();
    bool isValid() const;
    OA::OA_ptr<NameBoolPair> current() const;
    void reset();
  private:
    OA::OA_ptr<MySet> mSet;
    MySet::const_iterator mIter;
  };
  
private:
  OA::OA_ptr<MySet> mSet;
};

#endif
