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

#include "NameBoolDFSet.h"

using namespace OA;
using namespace OA::DataFlow;

NameBoolDFSet::NameBoolDFSet()
  : mSet()
{}

NameBoolDFSet::~NameBoolDFSet()
{}

OA_ptr<DataFlowSet> NameBoolDFSet::clone() const {
  assert(0);
}

bool NameBoolDFSet::operator==(DataFlowSet & orig_other) const {
  NameBoolDFSet & other = dynamic_cast<NameBoolDFSet &>(orig_other);

  if (mSet.size() != other.mSet.size()) {
    return false;
  }

  for (std::set<OA_ptr<NameBoolPair> >::const_iterator set1Iter = mSet.begin(); set1Iter != mSet.end(); ++set1Iter) {
    OA_ptr<NameBoolPair> cd1 = *set1Iter;
    std::set<OA_ptr<NameBoolPair> >::const_iterator set2Iter = other.mSet.find(cd1);
    if (set2Iter == other.mSet.end()) {
      return false;  // cd1 not found
    } else {
      OA_ptr<NameBoolPair> cd2 = *set2Iter;
      if (cd1 != cd2) {
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
  mSet.clear();
}

int NameBoolDFSet::size() const {
  return mSet.size();
}

bool NameBoolDFSet::isUniversalSet() const {
  throw new AnalysisException("Not yet implemented");
}

bool NameBoolDFSet::isEmpty() const {
  return mSet.empty();
}

void NameBoolDFSet::output(IRHandlesIRInterface & ir) const {
  throw new AnalysisException("Not yet implemented");
}

void NameBoolDFSet::dump(std::ostream & os, OA_ptr<IRHandlesIRInterface>) {
  throw new AnalysisException("Not yet implemented");
}

void NameBoolDFSet::insert(OA_ptr<NameBoolPair> element) {
  mSet.insert(element);
}

NameBoolDFSet::NameBoolPair::NameBoolPair(SEXP name, bool value)
  : mName(name), mValue(value)
{}

bool NameBoolDFSet::NameBoolPair::operator==(const NameBoolPair & other) {
  return (mName == other.mName);
}

bool NameBoolDFSet::NameBoolPair::operator<(const NameBoolPair & other) {
  return (mName < other.mName);
}
