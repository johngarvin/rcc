// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: VarSet.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "VarSet.h"

#include <support/DumpMacros.h>

#include <analysis/SideEffectAnnotationMap.h>
#include <analysis/Var.h>

namespace RAnnot {

// ----- typedefs for readability -----

typedef VarSet::MyVarT MyVarT;
typedef VarSet::MyVarSetT MyVarSetT;


// ----- constructor/destructor -----

VarSet::VarSet() {
}

VarSet::~VarSet() {
}


// ----- insertion and iterators -----

void VarSet::insert_use(const MyVarT & v) {
  assert(v->getUseDefType() == Var::Var_USE);
  m_uses.push_back(v);
}

void VarSet::insert_def(const MyVarT & v) {
  assert(v->getUseDefType() == Var::Var_DEF);
  m_defs.push_back(v);
}

std::list<Var *>::const_iterator VarSet::begin_uses() const {
  return m_uses.begin();
}

std::list<Var *>::const_iterator VarSet::end_uses() const {
  return m_uses.end();
}

std::list<Var *>::const_iterator VarSet::begin_defs() const {
  return m_defs.begin();
}

std::list<Var *>::const_iterator VarSet::end_defs() const {
  return m_defs.end();
}


// ----- handle for Annotation -----

PropertyHndlT VarSet::handle() {
  return SideEffectAnnotationMap::handle();
}


// ----- debugging -----

std::ostream& VarSet::dump(std::ostream& os) const {
  beginObjDump(os, VarSet);
  os << "Begin interprocedural uses:" << std::endl;
  for(std::list<Var *>::const_iterator it = begin_uses(); it != end_uses(); ++it) {
    (*it)->dump(os);
  }
  os << "End interprocedural uses" << std::endl;
  os << "Begin interprocedural defs:" << std::endl;
  for(std::list<Var *>::const_iterator it = begin_defs(); it != end_defs(); ++it) {
    (*it)->dump(os);
  }
  os << "End interprocedural uses" << std::endl;
  endObjDump(os, VarSet);
}

}  // end namespace RAnnot
