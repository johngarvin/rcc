// -*- Mode: C++ -*-
//
// Copyright (c) 2010 Rice University
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

// File: PreDebutSideEffect.cc
//
// Side effects, including uses and defs. Does not include defs that
// represent formal arguments. Used as an annotation type by
// PreDebutSideEffectAnnotationMap. Does not own the Var annotations
// it contains.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/PreDebutSideEffectAnnotationMap.h>

#include <support/DumpMacros.h>
#include <support/RccError.h>

#include "PreDebutSideEffect.h"

namespace RAnnot {

typedef PreDebutSideEffect::MyVarT MyVarT;
typedef PreDebutSideEffect::MyVarSetT MyVarSetT;
typedef PreDebutSideEffect::MyIteratorT MyIteratorT;

PreDebutSideEffect::PreDebutSideEffect(bool trivial, bool cheap)
  : m_side_effect(new SideEffect(trivial, cheap))
{
}

PreDebutSideEffect::~PreDebutSideEffect() {
}

// ----- trivial/cheap -----

bool PreDebutSideEffect::is_trivial() const {
  return m_side_effect->is_trivial();
}

bool PreDebutSideEffect::is_cheap() const {
  return m_side_effect->is_cheap();
}

void PreDebutSideEffect::set_action(bool x) {
  m_side_effect->set_action(x);
}

// ----- insertion -----

void PreDebutSideEffect::insert_use_sexp(const FuncInfo * fi, const SEXP sexp) {
  m_side_effect->insert_use_sexp(fi, sexp);
}

void PreDebutSideEffect::insert_def_sexp(const FuncInfo * fi, const SEXP sexp) {
  m_side_effect->insert_def_sexp(fi, sexp);
}


void PreDebutSideEffect::insert_use(const MyVarT & v) {
  m_side_effect->insert_use(v);
}

void PreDebutSideEffect::insert_def(const MyVarT & v) {
  m_side_effect->insert_def(v);
}

// ----- set operators -----

void PreDebutSideEffect::add(const SideEffect * x) {
  m_side_effect->add(x);
}

// ----- getters -----

bool PreDebutSideEffect::get_action() const {
  return m_side_effect->get_action();
}
  
MyVarSetT PreDebutSideEffect::get_uses() const {
  return m_side_effect->get_uses();
}

MyVarSetT PreDebutSideEffect::get_defs() const {
  return m_side_effect->get_defs();
}

// ----- iterators -----

MyIteratorT PreDebutSideEffect::begin_uses() const {
  return m_side_effect->begin_uses();
}

MyIteratorT PreDebutSideEffect::end_uses() const {
  return m_side_effect->end_uses();
}

MyIteratorT PreDebutSideEffect::begin_defs() const {
  return m_side_effect->begin_defs();
}

MyIteratorT PreDebutSideEffect::end_defs() const {
  return m_side_effect->end_defs();
}

// ----- ask whether this PreDebutSideEffect intersects with another -----

bool PreDebutSideEffect::intersects(SideEffect * other) const {
  return m_side_effect->intersects(other);
}

// ----- handle, cloning for Annotation -----

PropertyHndlT PreDebutSideEffect::handle() {
  return PreDebutSideEffectAnnotationMap::handle();
}

AnnotationBase * PreDebutSideEffect::clone() {
  rcc_error("Cloning not implemented in PreDebutSideEffect");
}

// ----- debugging -----

std::ostream & PreDebutSideEffect::dump(std::ostream & os) const {
  beginObjDump(os, PreDebutSideEffect);
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
  os << "Trivial: " << is_trivial() << std::endl;
  os << "Cheap: " << is_cheap() << std::endl;
  os << "Action: " << get_action() << std::endl;
  endObjDump(os, PreDebutSideEffect);
  return os;
}

SideEffect * PreDebutSideEffect::get_side_effect() const {
  return m_side_effect;
}

} // end namespace RAnnot
