// -*- Mode: C++ -*-
//
// Copyright (c) 2010 Rice University
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

// File: ExpressionDFSet.cc
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

#include "ExpressionDFSet.h"

using namespace OA;
using namespace OA::DataFlow;
using RAnnot::Var;

typedef ExpressionDFSet::ExpressionDFSetIterator ExpressionDFSetIterator;
typedef ExpressionDFSet::MySet MySet;

ExpressionDFSet::ExpressionDFSet()
  : mUniversal(false)
{
  mSet = new MySet();
}

ExpressionDFSet::~ExpressionDFSet()
{}

OA_ptr<DataFlowSet> ExpressionDFSet::clone() const {
  OA_ptr<ExpressionDFSet> clone; clone = new ExpressionDFSet();
  if (mUniversal) {
    clone->setUniversal();
  } else {
    OA_ptr<ExpressionDFSetIterator> iter; iter = getIterator();
    for ( ; iter->isValid(); ++*iter) {
      clone->insert(iter->current());
    }
  }
  return clone.convert<DataFlowSet>();
}

bool ExpressionDFSet::operator==(DataFlowSet & orig_other) const {
  ExpressionDFSet & other = dynamic_cast<ExpressionDFSet &>(orig_other);

  if (mUniversal && other.isUniversalSet()) {
    return true;
  } else if (mUniversal || other.isUniversalSet()) {
    return false;
  }

  if (mSet->size() != other.mSet->size()) {
    return false;
  }

  for (MySet::const_iterator set1Iter = mSet->begin(); set1Iter != mSet->end(); ++set1Iter) {
    SEXP cd1 = *set1Iter;
    MySet::const_iterator set2Iter = other.mSet->find(cd1);
    if (set2Iter == other.mSet->end()) {
      return false;  // cd1 not found
    }
  }  
  return true;
}

bool ExpressionDFSet::operator!=(DataFlowSet & orig_other) const {
  return !(*this == orig_other);
}

void ExpressionDFSet::setUniversal() {
  mUniversal = true;
}

void ExpressionDFSet::clear() {
  mSet->clear();
  mUniversal = false;
}

int ExpressionDFSet::size() const {
  if (mUniversal) {
    throw new AnalysisException("Don't know size of universal set");
  }
  return mSet->size();
}

bool ExpressionDFSet::isUniversalSet() const {
  return mUniversal;
}

bool ExpressionDFSet::isEmpty() const {
  return (!mUniversal && mSet->empty());
}

void ExpressionDFSet::output(IRHandlesIRInterface & ir) const {
  throw new AnalysisException("Not yet implemented");
}


void ExpressionDFSet::dump(std::ostream & os) {
  ExpressionDFSet::MySet::const_iterator iter;
  beginObjDump(os, ExpressionSet);
  if (mUniversal) {
    os << "UNIVERSAL SET" << std::endl;
  } else {
    for (iter = mSet->begin(); iter != mSet->end(); iter++) {
      SEXP exp = CAR(*iter);
      dumpSEXP(os, exp);
    }
  }
  endObjDump(os, ExpressionSet);
}

void ExpressionDFSet::dump(std::ostream & os, OA_ptr<IRHandlesIRInterface>) {
  dump(os);
}

void ExpressionDFSet::dump() {
  dump(std::cout);
}

void ExpressionDFSet::insert(SEXP element) {
  mSet->insert(element);
}

OA_ptr<ExpressionDFSet> ExpressionDFSet::meet(OA::OA_ptr<ExpressionDFSet> other) {
  OA_ptr<ExpressionDFSet> meet;
  if (this->isUniversalSet() || other->isUniversalSet()) {
    meet = new ExpressionDFSet();
    meet->setUniversal();
  } else {
    meet = this->clone().convert<ExpressionDFSet>();
    OA_ptr<ExpressionDFSet::ExpressionDFSetIterator> iter;
    for (iter = other->getIterator(); iter->isValid(); ++(*iter)) {
      meet->insert(iter->current());
    }
  }
  return meet;
}

OA_ptr<ExpressionDFSetIterator> ExpressionDFSet::getIterator() const {
  if (mUniversal) {
    throw new AnalysisException("Can't iterate over universal set");
  }
  OA_ptr<ExpressionDFSetIterator> it;
  it = new ExpressionDFSetIterator(mSet);
  return it;
}

bool ExpressionDFSet::lookup(SEXP e) const {
  if (mUniversal) {
    return true;
  }
  MySet::const_iterator iter = mSet->find(e);
  return (iter != mSet->end());
}

// ----- ExpressionDFSetIterator methods -----

ExpressionDFSetIterator::ExpressionDFSetIterator(OA_ptr<MySet> set)
  : mSet(set)
{
  assert(!mSet.ptrEqual(NULL));
  mIter = mSet->begin();
}

ExpressionDFSetIterator::~ExpressionDFSetIterator()
{}

void ExpressionDFSetIterator::operator++() {
  if (isValid()) mIter++;
}

/// is the iterator at the end
bool ExpressionDFSetIterator::isValid() const {
  return (mIter != mSet->end());
}

/// return copy of current node in iterator
SEXP ExpressionDFSetIterator::current() const {
  assert(isValid());
  return (*mIter);
}

/// reset iterator to beginning of set
void ExpressionDFSetIterator::reset() {
  mIter = mSet->begin();
}
