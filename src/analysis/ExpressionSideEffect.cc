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

// File: ExpressionSideEffect.cc
//
// Side effects, including uses and defs. Does not include defs that
// represent formal arguments. Used as an annotation type by
// ExpressionSideEffectAnnotationMap. Does not own the Var annotations
// it contains.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/ExpressionSideEffectAnnotationMap.h>

#include <support/DumpMacros.h>
#include <support/RccError.h>

#include "ExpressionSideEffect.h"

namespace RAnnot {

typedef ExpressionSideEffect::MyVarT MyVarT;
typedef ExpressionSideEffect::MyVarSetT MyVarSetT;
typedef ExpressionSideEffect::MyIteratorT MyIteratorT;

ExpressionSideEffect::ExpressionSideEffect(bool trivial, bool cheap)
  : m_side_effect(new SideEffect(trivial, cheap))
{
}

ExpressionSideEffect::~ExpressionSideEffect() {
}

// ----- trivial/cheap -----

bool ExpressionSideEffect::is_trivial() const {
  return m_side_effect->is_trivial();
}

bool ExpressionSideEffect::is_cheap() const {
  return m_side_effect->is_cheap();
}

void ExpressionSideEffect::set_action(bool x) {
  m_side_effect->set_action(x);
}

// ----- insertion -----

void ExpressionSideEffect::insert_use_sexp(const FuncInfo * fi, const SEXP sexp) {
  m_side_effect->insert_use_sexp(fi, sexp);
}

void ExpressionSideEffect::insert_def_sexp(const FuncInfo * fi, const SEXP sexp) {
  m_side_effect->insert_def_sexp(fi, sexp);
}


void ExpressionSideEffect::insert_use(const MyVarT & v) {
  m_side_effect->insert_use(v);
}

void ExpressionSideEffect::insert_def(const MyVarT & v) {
  m_side_effect->insert_def(v);
}

// ----- set operators -----

void ExpressionSideEffect::add(const SideEffect * x) {
  m_side_effect->add(x);
}

// ----- getters -----

bool ExpressionSideEffect::get_action() const {
  return m_side_effect->get_action();
}
  
MyVarSetT ExpressionSideEffect::get_uses() const {
  return m_side_effect->get_uses();
}

MyVarSetT ExpressionSideEffect::get_defs() const {
  return m_side_effect->get_defs();
}

// ----- iterators -----

MyIteratorT ExpressionSideEffect::begin_uses() const {
  return m_side_effect->begin_uses();
}

MyIteratorT ExpressionSideEffect::end_uses() const {
  return m_side_effect->end_uses();
}

MyIteratorT ExpressionSideEffect::begin_defs() const {
  return m_side_effect->begin_defs();
}

MyIteratorT ExpressionSideEffect::end_defs() const {
  return m_side_effect->end_defs();
}

// ----- ask whether this SideEffect intersects with another -----

bool ExpressionSideEffect::intersects(SideEffect * other) const {
  return m_side_effect->intersects(other);
}

// ----- handle, cloning for Annotation -----

PropertyHndlT ExpressionSideEffect::handle() {
  return ExpressionSideEffectAnnotationMap::handle();
}

AnnotationBase * ExpressionSideEffect::clone() {
  rcc_error("Cloning not implemented in ExpressionSideEffect");
}

// ----- debugging -----

std::ostream & ExpressionSideEffect::dump(std::ostream & os) const {
  beginObjDump(os, ExpressionSideEffect);
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
  endObjDump(os, ExpressionSideEffect);
  return os;
}

SideEffect * ExpressionSideEffect::get_side_effect() const {
  return m_side_effect;
}

} // end namespace RAnnot
