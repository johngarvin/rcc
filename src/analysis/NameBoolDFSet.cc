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

// File: NameBoolDFSet.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <set>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/LocalityType.h>
#include <analysis/VarRefFactory.h>
#include <analysis/Utils.h>
#include <analysis/Var.h>

#include <support/DumpMacros.h>

#include "NameBoolDFSet.h"

using namespace OA;
using namespace OA::DataFlow;
using RAnnot::Var;

typedef NameBoolDFSet::NameBoolDFSetIterator NameBoolDFSetIterator;
typedef NameBoolDFSet::MySet MySet;

NameBoolDFSet::NameBoolDFSet()
{
  mSet = new MySet();
}

NameBoolDFSet::~NameBoolDFSet()
{}

OA_ptr<DataFlowSet> NameBoolDFSet::clone() const {
  OA_ptr<NameBoolDFSet> clone; clone = new NameBoolDFSet();
  OA_ptr<NameBoolDFSetIterator> iter; iter = getIterator();
  for ( ; iter->isValid(); ++*iter) {
    clone->insert(iter->current());
  }
  return clone.convert<DataFlowSet>();
}

bool NameBoolDFSet::operator==(DataFlowSet & orig_other) const {
  NameBoolDFSet & other = dynamic_cast<NameBoolDFSet &>(orig_other);

  if (mSet->size() != other.mSet->size()) {
    return false;
  }

  for (std::set<OA_ptr<NameBoolPair> >::const_iterator set1Iter = mSet->begin(); set1Iter != mSet->end(); ++set1Iter) {
    OA_ptr<NameBoolPair> cd1 = *set1Iter;
    std::set<OA_ptr<NameBoolPair> >::const_iterator set2Iter = other.mSet->find(cd1);
    if (set2Iter == other.mSet->end()) {
      return false;  // cd1 not found
    } else {
      OA_ptr<NameBoolPair> cd2 = *set2Iter;
      if (!(cd1->equiv(*cd2))) {
	return false;
      }
    }
  }  
  return true;
}

bool NameBoolDFSet::operator!=(DataFlowSet & orig_other) const {
  return !(*this == orig_other);
}

void NameBoolDFSet::setUniversal() {
  throw new AnalysisException("Not yet implemented");
}

void NameBoolDFSet::clear() {
  mSet->clear();
}

int NameBoolDFSet::size() const {
  return mSet->size();
}

bool NameBoolDFSet::isUniversalSet() const {
  throw new AnalysisException("Not yet implemented");
}

bool NameBoolDFSet::isEmpty() const {
  return mSet->empty();
}

void NameBoolDFSet::output(IRHandlesIRInterface & ir) const {
  throw new AnalysisException("Not yet implemented");
}


void NameBoolDFSet::dump(std::ostream & os) {
  NameBoolDFSet::MySet::const_iterator iter;

  beginObjDump(os, NameBoolSet);
  for (iter = mSet->begin(); iter != mSet->end(); iter++) {
    SEXP name = (*iter)->getName()->get_name();
    bool value = (*iter)->getValue();
    dumpSEXP(os, name);
    dumpVar(os, value);
  }
  endObjDump(os, NameBoolSet);
}

void NameBoolDFSet::dump(std::ostream & os, OA_ptr<IRHandlesIRInterface>) {
  dump(os);
}

void NameBoolDFSet::dump() {
  dump(std::cout);
}

void NameBoolDFSet::insert(OA_ptr<NameBoolPair> element) {
  mSet->insert(element);
}

OA_ptr<NameBoolDFSet> NameBoolDFSet::meet(OA::OA_ptr<NameBoolDFSet> other) {
  OA_ptr<NameBoolDFSet> meet; meet = this->clone().convert<NameBoolDFSet>();
  OA_ptr<NameBoolDFSet::NameBoolDFSetIterator> iter;
  for (iter = other->getIterator(); iter->isValid(); ++(*iter)) {
    if (mSet->find(iter->current()) == mSet->end()) {
      meet->insert(iter->current());
    } else if (iter->current()->getValue() == true) {
      meet->replace(iter->current()->getName(), iter->current()->getValue());
    }
  }
  return meet;
}

OA_ptr<NameBoolDFSetIterator> NameBoolDFSet::getIterator() const {
  OA_ptr<NameBoolDFSetIterator> it;
  it = new NameBoolDFSetIterator(mSet);
  return it;
}

bool NameBoolDFSet::lookup(OA_ptr<R_VarRef> e) const {
  OA_ptr<NameBoolPair> find_pair; find_pair = new NameBoolPair(e, false);
  NameBoolDFSet::MySet::const_iterator iter = mSet->find(find_pair);
  if (iter != mSet->end()) {
    return (*iter)->getValue();
  } else {
    return false;
  }
}

void NameBoolDFSet::replace(OA_ptr<R_VarRef> e, bool value) {
  OA_ptr<NameBoolPair> pair; pair = new NameBoolPair(e, value);
  NameBoolDFSet::MySet::const_iterator iter = mSet->find(pair);
  if (iter != mSet->end()) {
    mSet->erase(*iter);
    mSet->insert(pair);
  }
}

// propagate(value, expression): propagate lattice value from expression to subexpressions
// TODO: better description
void NameBoolDFSet::propagate(OA_ptr<NameBoolDFSet> in, PredicateType predicate, bool value, SEXP cell) {
  VarRefFactory * const fact = VarRefFactory::get_instance();
  SEXP e = CAR(cell);
  if (is_symbol(e) && getProperty(Var, cell)->getScopeType() == Locality::Locality_LOCAL) {
    in->replace(fact->make_body_var_ref(cell), value);
  } else if (is_call(e)) {
    int nargs = Rf_length(call_args(e));
    for (int i=1; i<=nargs; i++) {
      if ((*predicate)(e, i)) {
	propagate(in, predicate, value, call_nth_arg_c(e, i));
      }
    }
  }
}

// ----- NameBoolPair methods -----

NameBoolDFSet::NameBoolPair::NameBoolPair(OA_ptr<R_VarRef> name, bool value)
  : mName(name), mValue(value)
{}

bool NameBoolDFSet::NameBoolPair::operator==(const NameBoolPair & other) {
  return (mName == other.mName);
}

bool NameBoolDFSet::NameBoolPair::equiv(const NameBoolPair & other) {
  return (mName == other.mName && mValue == other.mValue);
}

bool NameBoolDFSet::NameBoolPair::operator<(const NameBoolPair & other) {
  return (mName < other.mName);
}

OA_ptr<R_VarRef> NameBoolDFSet::NameBoolPair::getName() {
  return mName;
}

bool NameBoolDFSet::NameBoolPair::getValue() {
  return mValue;
}

// ----- NameBoolDFSetIterator methods -----

NameBoolDFSetIterator::NameBoolDFSetIterator(OA_ptr<std::set<OA_ptr<NameBoolPair> > > set)
  : mSet(set)
{
  assert(!mSet.ptrEqual(NULL));
  mIter = mSet->begin();
}

NameBoolDFSetIterator::~NameBoolDFSetIterator()
{}

void NameBoolDFSetIterator::operator++() {
  if (isValid()) mIter++;
}

/// is the iterator at the end
bool NameBoolDFSetIterator::isValid() const {
  return (mIter != mSet->end());
}

/// return copy of current node in iterator
OA_ptr<NameBoolDFSet::NameBoolPair> NameBoolDFSetIterator::current() const {
  assert(isValid());
  return (*mIter);
}

/// reset iterator to beginning of set
void NameBoolDFSetIterator::reset() {
  mIter = mSet->begin();
}
