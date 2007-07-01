// -*- Mode: C++ -*-
//
// Copyright (c) 2007 Rice University
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

// File: ScopeAnnotationMap.cc
//
// Maps cons cells (SEXPs of LISTSXP or LANGSXP type) to the FuncInfo
// of the scope where they reside.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FuncInfo.h>
#include <analysis/SimpleIterators.h>
#include <analysis/Utils.h>

#include <support/RccError.h>

#include "ScopeAnnotationMap.h"

namespace RAnnot {

// ----- type definitions for readability -----

typedef ScopeAnnotationMap::MyKeyT MyKeyT;
typedef ScopeAnnotationMap::MyMappedT MyMappedT;
typedef ScopeAnnotationMap::iterator iterator;
typedef ScopeAnnotationMap::const_iterator const_iterator;

//  ----- constructor/destructor ----- 
  
ScopeAnnotationMap::ScopeAnnotationMap()
  : m_computed(false),
    m_map()
  {}

ScopeAnnotationMap::~ScopeAnnotationMap() {
  // no-op; does not own annotations in map
}

// ----- singleton pattern -----

ScopeAnnotationMap * ScopeAnnotationMap::get_instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT ScopeAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

void ScopeAnnotationMap::create() {
  s_instance = new ScopeAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

ScopeAnnotationMap * ScopeAnnotationMap::s_instance = 0;
PropertyHndlT ScopeAnnotationMap::s_handle = "Scope";

//  ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// TODO: delete this when fully refactored to disallow insertion from outside.
MyMappedT & ScopeAnnotationMap::operator[](const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }

  return m_map[k];
}

// Perform the computation if necessary and returns the requested
// data.
MyMappedT ScopeAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    rcc_error("Possible invalid key not found in map");
  }

  return annot->second;
}
  
bool ScopeAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator ScopeAnnotationMap::begin() { return m_map.begin(); }
iterator ScopeAnnotationMap::end() { return m_map.end(); }
const_iterator ScopeAnnotationMap::begin() const { return m_map.begin(); }
const_iterator ScopeAnnotationMap::end() const { return m_map.end(); }

// ----- computation -----

void ScopeAnnotationMap::compute() {
  R_Analyst * an = R_Analyst::get_instance();
  FuncInfoIterator fii(an->get_scope_tree_root());
  for(fii.Reset(); fii.IsValid(); ++fii) {
    FuncInfo * scope = fii.Current();
    R_PreorderIterator iter(scope->get_defn());
    for(iter.reset(); iter.isValid(); ++iter) {
      if (is_cons(iter.current())) {
	m_map[iter.current()] = scope;
      }
    }
  }
}

} // namespace RAnnot
