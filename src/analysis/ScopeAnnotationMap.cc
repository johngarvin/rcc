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
  
ScopeAnnotationMap::ScopeAnnotationMap() {}

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

// ----- computation -----

void ScopeAnnotationMap::compute() {
  FuncInfo * scope;
  FOR_EACH_PROC(scope) {
    R_PreorderIterator iter(scope->get_sexp());
    for(iter.reset(); iter.isValid(); ++iter) {
      if (is_cons(iter.current())) {
	get_map()[iter.current()] = scope;
      }
    }
  }
}

} // namespace RAnnot
