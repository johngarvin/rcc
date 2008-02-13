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
typedef SideEffect::MyVarSetT MyVarSetT;


// ----- constructor/destructor -----

SideEffect::SideEffect() {
}

SideEffect::~SideEffect() {
}


// ----- insertion -----

void SideEffect::insert_use(const FuncInfo * fi, const Var * v) {
  assert(v->getUseDefType() == Var::Var_USE);
  OA::ProcHandle proc = make_proc_h(fi->get_defn());
  OA::SymHandle symbol = make_sym_h(SymbolTableFacade::get_instance()->find_entry(fi, v));
  insert_use(R_Analyst::get_instance()->get_interface()->getLocation(proc, symbol));
}

void SideEffect::insert_use(const MyVarT & v) {
  m_uses.insert(v);
}

void SideEffect::insert_def(const FuncInfo * fi, const Var * v) {
  assert(v->getUseDefType() == Var::Var_DEF);
  OA::ProcHandle proc = make_proc_h(fi->get_defn());
  OA::SymHandle symbol = make_sym_h(SymbolTableFacade::get_instance()->find_entry(fi, v));
  insert_def(R_Analyst::get_instance()->get_interface()->getLocation(proc, symbol));
}

void SideEffect::insert_def(const MyVarT & v) {
  m_defs.insert(v);
}


// ----- iterators -----

MyVarSetT::const_iterator SideEffect::begin_uses() const {
  return m_uses.begin();
}

MyVarSetT::const_iterator SideEffect::end_uses() const {
  return m_uses.end();
}

MyVarSetT::const_iterator SideEffect::begin_defs() const {
  return m_defs.begin();
}

MyVarSetT::const_iterator SideEffect::end_defs() const {
  return m_defs.end();
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
  os << "Begin interprocedural uses:" << std::endl;
  for(MyVarSetT::const_iterator it = begin_uses(); it != end_uses(); ++it) {
    (*it)->dump(os);
  }
  os << "End interprocedural uses" << std::endl;
  os << "Begin interprocedural defs:" << std::endl;
  for(MyVarSetT::const_iterator it = begin_defs(); it != end_defs(); ++it) {
    (*it)->dump(os);
  }
  os << "End interprocedural uses" << std::endl;
  endObjDump(os, SideEffect);
}

}  // end namespace RAnnot
