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

// File: SideEffect.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "SideEffect.h"

#include <support/DumpMacros.h>
#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/HandleInterface.h>
#include <analysis/SideEffectAnnotationMap.h>
#include <analysis/SymbolTableFacade.h>
#include <analysis/Var.h>
#include <analysis/VarInfo.h>

using namespace HandleInterface;

namespace RAnnot {

// ----- typedefs for readability -----

typedef SideEffect::MyVarT MyVarT;
typedef SideEffect::MyRawVarSetT MyRawVarSetT;
typedef SideEffect::MyVarSetT MyVarSetT;
typedef SideEffect::MyIteratorT MyIteratorT;

// ----- static functions -----

static bool sets_intersect(MyVarSetT s, MyVarSetT t);

// ----- constructor/destructor -----

SideEffect::SideEffect() : m_trivial(false),
			   m_cheap(false)
{
}

SideEffect::~SideEffect() {
}

// ----- trivial/cheap -----

  void SideEffect::set_trivial(bool x) {
    m_trivial = x;
  }

  bool SideEffect::is_trivial() {
    return m_trivial;
  }

  void SideEffect::set_cheap(bool x) {
    m_cheap = x;
  }

  bool SideEffect::is_cheap() {
    return m_cheap;
  }

  void SideEffect::set_action(bool x) {
    m_action = x;
  }

// ----- insertion -----

void SideEffect::insert_use_var(const FuncInfo * fi, const UseVar * use) {
  OA::ProcHandle proc = make_proc_h(fi->get_sexp());
  OA::SymHandle symbol = make_sym_h(SymbolTableFacade::get_instance()->find_entry(fi, use));
  insert_use(make_var_info(symbol));
}

void SideEffect::insert_def_var(const FuncInfo * fi, const DefVar * def) {
  OA::ProcHandle proc = make_proc_h(fi->get_sexp());
  OA::SymHandle symbol = make_sym_h(SymbolTableFacade::get_instance()->find_entry(fi, def));
  insert_def(make_var_info(symbol));
}

void SideEffect::insert_use(const MyVarT & v) {
  m_uses.insert(v);
}

void SideEffect::insert_def(const MyVarT & v) {
  m_defs.insert(v);
}

  // ----- set operators -----

void SideEffect::add(const SideEffect * x) {
  MyIteratorT it;
  for(it = x->begin_uses(); it != x->end_uses(); ++it) {
    m_uses.insert(*it);
  }
  for(it = x->begin_defs(); it != x->end_defs(); ++it) {
    m_defs.insert(*it);
  }
}

// ----- getters -----

bool SideEffect::get_action() {
  return m_action;
}
  
MyVarSetT SideEffect::get_uses() {
  return m_uses;
}

MyVarSetT SideEffect::get_defs() {
  return m_defs;
}

// ----- iterators -----

MyIteratorT SideEffect::begin_uses() const {
  return m_uses.begin();
}

MyIteratorT SideEffect::end_uses() const {
  return m_uses.end();
}

MyIteratorT SideEffect::begin_defs() const {
  return m_defs.begin();
}

MyIteratorT SideEffect::end_defs() const {
  return m_defs.end();
}

// ----- ask whether this SideEffect intersects with another -----

bool sets_intersect(MyVarSetT s, MyVarSetT t) {
  for(MyIteratorT iter = s.begin(); iter != s.end(); ++iter) {
    if (t.find(*iter) != t.end()) {
      return true;
    }
  }
  return false;
}

bool SideEffect::intersects(SideEffect * other) {
  if (get_action() || other->get_action()) {
    return true;
  }

  bool true_dep = sets_intersect(get_defs(), other->get_uses());
  bool anti_dep = sets_intersect(get_uses(), other->get_defs());
  bool output_dep = sets_intersect(get_defs(), other->get_defs());
  return (true_dep || anti_dep || output_dep);
}

// ----- handle, cloning for Annotation -----

PropertyHndlT SideEffect::handle() {
  return SideEffectAnnotationMap::handle();
}

AnnotationBase * SideEffect::clone() {
  rcc_error("Cloning not implemented in SideEffect");
}

// ----- debugging -----

std::ostream& SideEffect::dump(std::ostream& os) const {
  beginObjDump(os, SideEffect);
  os << "Begin uses:" << std::endl;
  for(MyIteratorT it = begin_uses(); it != end_uses(); ++it) {
    (*it)->dump(os);
  }
  os << "End uses" << std::endl;
  os << "Begin defs:" << std::endl;
  for(MyIteratorT it = begin_defs(); it != end_defs(); ++it) {
    (*it)->dump(os);
  }
  os << "End defs" << std::endl;
  endObjDump(os, SideEffect);
}

}  // end namespace RAnnot
