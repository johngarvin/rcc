// -*- Mode: C++ -*-

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

// File: VarBinding.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

// ----- includes -----

#include <analysis/Analyst.h>
#include <analysis/AnalysisResults.h>
#include <analysis/SymbolTable.h>
#include <analysis/LexicalScope.h>
#include <analysis/VarBindingAnnotationMap.h>
#include <analysis/VarInfo.h>
#include <support/DumpMacros.h>

#include "VarBinding.h"

// ----- definitions -----

namespace RAnnot {

/// constructor
VarBinding::VarBinding()
  : m_scopes()
  {}

// ----- iterator methods -----

  VarBinding::const_iterator VarBinding::begin() const {
    return m_scopes.begin();
  }

  VarBinding::const_iterator VarBinding::end() const {
    return m_scopes.end();
  }

  void VarBinding::insert(KeyT key) {
    m_scopes.push_back(key);
  }
  
  PropertyHndlT VarBinding::handle() {
    return VarBindingAnnotationMap::handle();
  }

  bool VarBinding::is_single() {
    MyContainerT::const_iterator i = begin();
    if (i == end()) {
      return false;
    }
    return (++i == end());
  }

  bool VarBinding::is_global() {
    // global if m_scopes has exactly one element and it's the global
    // scope.
    static LexicalScope * global = R_Analyst::get_instance()->get_global_scope();
    return (is_single() && *begin() == global);
  }

  bool VarBinding::is_internal() {
    // internal (library, builtin, or special) if m_scopes has exactly
    // one element and it's the library scope.
    static LexicalScope * library_scope = R_Analyst::get_instance()->get_library_scope();
    return (is_single() && *begin() == library_scope);
  }

  bool VarBinding::is_unbound() {
    return (begin() == end());
  }

  std::string VarBinding::get_location(SEXP name, SubexpBuffer * sb) {
    FundefLexicalScope * scope;
    // If there's only one binding and it's in a fundef, then look up
    // the var in the symbol table and get a location for it.
    if (is_single() && (scope = dynamic_cast<FundefLexicalScope *>(*begin())) != 0) {
      SymbolTable * st = getProperty(SymbolTable, scope->get_fundef());
      VarInfo * vi = (*st)[name];
      assert(vi);
      return vi->get_location(sb); // gets a location; creates it if necessary
    } else {
      return "";
    }
  }

  AnnotationBase * VarBinding::clone() {
    VarBinding * vb = new VarBinding();
    for(MyContainerT::const_iterator i = begin(); i != end(); ++i) {
      vb->insert(*i);
    }
    return vb;
  }

  // debug
  std::ostream & VarBinding::dump(std::ostream & os) const {
    os << "VarBinding {";
    // print out each binding scope in sequence
    for (MyContainerT::const_iterator s = begin(); s != end(); ++s) {
      dumpPtr(os, *s);
    }
    os << "}\n";
    return os;
  }
}
