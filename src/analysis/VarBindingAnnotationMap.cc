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

// File: VarBindingAnnotationMap.h
//
// Maps each variable to a VarBinding that describes its binding
// scopes.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <vector>

#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertySet.h>
#include <analysis/Utils.h>
#include <analysis/Var.h>
#include <analysis/VarBinding.h>
#include <analysis/VarBindingAnnotationMap.h>

#include "VarBindingAnnotationMap.h"

using namespace RAnnot;
using namespace HandleInterface;

// ----- forward declarations of file-scope functions

static bool defined_local_in_scope(Var * v, FuncInfo * s);

namespace RAnnot {

// ----- typedefs for readability -----

typedef VarBindingAnnotationMap::MyKeyT MyKeyT;
typedef VarBindingAnnotationMap::MyMappedT MyMappedT;
typedef VarBindingAnnotationMap::iterator iterator;
typedef VarBindingAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor -----

VarBindingAnnotationMap::VarBindingAnnotationMap(bool ownsAnnotations /* = true */)
  : m_computed(false),
    m_map()
  {}

VarBindingAnnotationMap::~VarBindingAnnotationMap()
  {}

// ----- singleton pattern -----

VarBindingAnnotationMap * VarBindingAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT VarBindingAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void VarBindingAnnotationMap::create() {
  m_instance = new VarBindingAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

VarBindingAnnotationMap * VarBindingAnnotationMap::m_instance = 0;
PropertyHndlT VarBindingAnnotationMap::m_handle = "VarBinding";

// ----- demand-driven analysis -----

MyMappedT & VarBindingAnnotationMap::operator[](const MyKeyT & k) {
  rcc_error("VarBindingAnnotationMap::operator[] not implemented");
}

MyMappedT VarBindingAnnotationMap::get(const MyKeyT & k) {
  if (!m_computed) {
    compute();
    m_computed = true;
  }

  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    rcc_error("Possible invalid key not found in VarBinding map");
  }

  return annot->second;
}

bool VarBindingAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator VarBindingAnnotationMap::begin() { return m_map.begin(); }
iterator VarBindingAnnotationMap::end() { return m_map.end(); }
const_iterator VarBindingAnnotationMap::begin() const { return m_map.begin(); }
const_iterator VarBindingAnnotationMap::end() const { return m_map.end(); }

// ----- computation -----

/// Each Var is bound in some subset of the sequence of ancestor
/// scopes of its containing function. Compute the subsequence of
/// scopes in which each Var is bound and add the (Var,sequence) pair
/// into the annotation map.
void VarBindingAnnotationMap::compute() {
  static FuncInfo * global = R_Analyst::get_instance()->get_scope_tree_root();
  // for each function
  FuncInfoIterator fii(global);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo * fi = fii.Current();
    assert(fi != 0);
    // each mention
    FuncInfo::mention_iterator mi;
    for (mi = fi->begin_mentions(); mi != fi->end_mentions(); ++mi) {
      Var * v = *mi;
      v = getProperty(Var, v->getMention_c());
      // FIXME: should make sure we always get the data-flow-solved
      // version of the Var. Shouldn't have to loop through
      // getProperty!
      VarBinding * scopes = new VarBinding();
      bool binding_found;
      switch(v->getScopeType()) {
      case Locality_LOCAL:
	// bound in current scope only
	scopes->insert(fi->get_scope());
	break;
      case Locality_BOTTOM:
	// bound in current scope and one or more ancestors
	scopes->insert(fi->get_scope());
	// FALLTHROUGH
      case Locality_FREE:
	// start at this scope's parent; iterate upward through ancestors
	binding_found = false;
	for(FuncInfo * a = fi->Parent(); a != 0; a = a->Parent()) {
	  if (defined_local_in_scope(v,a)) {
	    scopes->insert(a->get_scope());
	    binding_found = true;
	  }
	}

	// for R internal names, add the library scope
	if (is_library(CAR(v->getMention_c()))) {
	  scopes->insert(R_Analyst::get_instance()->get_library_scope());
	}

	// double-arrow (<<-) definitions declare the name in the
	// global scope if it's not local in some middle scope. Just
	// check for a def; since we're here, we already know the
	// mention is non-local.
	if (v->getUseDefType() == Var::Var_DEF) {
	  scopes->insert(R_Analyst::get_instance()->get_global_scope());
	}

	break;
      default:
	rcc_error("Unknown scope type encountered");
	break;
      }
      // whether global or not...
      m_map[make_sym_h(v->getMention_c())] = scopes;
    }
  }
}
  
  
}

/// Is the given variable defined as local in the given scope?
static bool defined_local_in_scope(Var * v, FuncInfo * s) {
  FuncInfo::mention_iterator mi;
  for(mi = s->begin_mentions(); mi != s->end_mentions(); ++mi) {
    Var * m = *mi;
    if (m->getUseDefType() == Var::Var_DEF  &&
	m->getScopeType() == Locality_LOCAL &&
	m->getName() == v->getName())
    {
      return true;
    }
  }
  return false;
}
