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
// Annotates a mention with binding information: the list of scopes in
// which the mention may be bound locally. (Most mentions are bound in
// only one scope, so the list will commonly be a singleton.)
//
// Author: John Garvin (garvin@cs.rice.edu)

// ----- includes -----

#include <analysis/Analyst.h>
#include <analysis/VarBindingAnnotationMap.h>
#include <support/DumpMacros.h>

#include "VarBinding.h"

// ----- definitions -----

namespace RAnnot {

/// constructor
VarBinding::VarBinding()
  : m_scopes()
  {}

// ----- iterator methods -----

  VarBinding::const_iterator VarBinding::begin() {
    return m_scopes.begin();
  }

  VarBinding::const_iterator VarBinding::end() {
    return m_scopes.end();
  }

  void VarBinding::insert(KeyT key) {
    m_scopes.push_back(key);
  }
  
  PropertyHndlT VarBinding::handle() {
    return VarBindingAnnotationMap::handle();
  }

  bool VarBinding::is_global() {
    // global if m_scopes has exactly one element and it's the global
    // scope.
    MyContainerT::iterator s1, s2;
    static FuncInfo * global = R_Analyst::get_instance()->get_scope_tree_root();
    s1 = s2 = m_scopes.begin();
    assert(s1 != m_scopes.end());    // empty scope list is an error
    if (*s1 != global) {
      return false;
    }
    // At this point s1 is the global scope.
    // Ensure only one element; any scope above global is an error
    assert(++s2 == m_scopes.end());
    return true;
  }

  AnnotationBase * VarBinding::clone() {
    VarBinding * vb = new VarBinding();
    for(MyContainerT::iterator i = m_scopes.begin(); i != m_scopes.end(); ++i) {
      vb->insert(*i);
    }
    return vb;
  }

  // debug
  std::ostream & VarBinding::dump(std::ostream & os) const {
    os << "VarBinding {";
    // print out each binding scope in sequence
    for (MyContainerT::const_iterator s = m_scopes.begin(); s != m_scopes.end(); ++s) {
      dumpPtr(os, *s);
    }
    os << "}\n";
    return os;
  }
}
