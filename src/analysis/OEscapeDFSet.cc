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

// File: OEscapeDFSet.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <assert.h>

#include <analysis/AnalysisException.h>

#include "OEscapeDFSet.h"

using namespace OA;
using namespace OA::DataFlow;

typedef OEscapeDFSet::ProcSetElement ProcSetElement;
typedef OEscapeDFSet::VarSetElement VarSetElement;

OEscapeDFSet::OEscapeDFSet() {
  // note: regular initialization doesn't work here because of OA_ptr template
  mProcSet = new MyProcSet;
  mVarSet = new MyVarSet;
}

OEscapeDFSet::~OEscapeDFSet()
{}

OA_ptr<DataFlowSet> OEscapeDFSet::clone() const {
  assert(0);
}

bool OEscapeDFSet::operator==(DataFlowSet & orig_other) const {
  OEscapeDFSet & other = dynamic_cast<OEscapeDFSet &>(orig_other);
  
  if (mProcSet->size() != other.mProcSet->size() ||
      mVarSet->size() != other.mVarSet->size())
  {
    return false;
  }

  // same size:  for every element in our set, find the element in other.set
  
  for (MyProcSet::const_iterator set1Iter = mProcSet->begin(); set1Iter != mProcSet->end(); ++set1Iter) {
    OA_ptr<ProcSetElement> cd1 = *set1Iter;
    MyProcSet::const_iterator set2Iter = other.mProcSet->find(cd1);
    if (set2Iter == other.mProcSet->end()) {
      return false;  // cd1 not found
    } else {
      OA_ptr<ProcSetElement> cd2 = *set2Iter;
      if (cd1 != cd2) {
	return false;
      }
    }
  }

  for (MyVarSet::const_iterator set1Iter = mVarSet->begin(); set1Iter != mVarSet->end(); ++set1Iter) {
    OA_ptr<VarSetElement> cd1 = *set1Iter;
    MyVarSet::const_iterator set2Iter = other.mVarSet->find(cd1);
    if (set2Iter == other.mVarSet->end()) {
      return false;  // cd1 not found
    } else {
      OA_ptr<VarSetElement> cd2 = *set2Iter;
      if (cd1 != cd2) {
	return false;
      }
    }
  }

  // if we get here, all elements of set1 and set2 are the same
  return true;
}

bool OEscapeDFSet::operator!=(DataFlowSet & orig_other) const {
  return !(*this == orig_other);
}

void OEscapeDFSet::setUniversal() {
  throw new AnalysisException("Not yet implemented");
}

void OEscapeDFSet::clear() {
  mProcSet->clear();
  mVarSet->clear();
}

int OEscapeDFSet::size() const {
  return mProcSet->size() + mVarSet->size();
}

bool OEscapeDFSet::isUniversalSet() const {
  throw new AnalysisException("Not yet implemented");
}

bool OEscapeDFSet::isEmpty() const {
  return (mProcSet->empty() && mVarSet->empty());
}

void OEscapeDFSet::output(IRHandlesIRInterface & ir) const {
  throw new AnalysisException("Not yet implemented");
}

void OEscapeDFSet::dump(std::ostream & os, OA_ptr<IRHandlesIRInterface>) {
  throw new AnalysisException("Not yet implemented");
}

void OEscapeDFSet::insertProc(OA_ptr<ProcSetElement> proc) {
  mProcSet->insert(proc);
}

void OEscapeDFSet::insertVar(OA_ptr<VarSetElement> var) {
  mVarSet->insert(var);
}

// ----- ProcSetElement -----

OEscapeDFSet::ProcSetElement::ProcSetElement(OA::ProcHandle proc, bool fresh)
  : mProc(proc), mFresh(fresh)
{}

bool OEscapeDFSet::ProcSetElement::operator==(const ProcSetElement & other) {
  return (mProc == other.mProc);
}

bool OEscapeDFSet::ProcSetElement::operator<(const ProcSetElement & other) {
  return (mProc < other.mProc);
}

// ----- VarSetElement -----

OEscapeDFSet::VarSetElement::VarSetElement(SSA::SSAStandard::Def * var,
					   bool fresh,
					   bool escaped,
					   bool returned)
  : mVar(var), mVFresh(fresh), mEscaped(escaped), mReturned(returned)
{}

bool OEscapeDFSet::VarSetElement::operator==(const VarSetElement & other) {
  return (mVar == other.mVar);
}

bool OEscapeDFSet::VarSetElement::operator<(const VarSetElement & other) {
  return (mVar < other.mVar);
}
