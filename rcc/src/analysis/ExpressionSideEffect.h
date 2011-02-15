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

// File: ExpressionSideEffect.h
//
// Side effects, including uses and defs. Does not include defs that
// represent formal arguments. Used as an annotation type by
// ExpressionSideEffectAnnotationMap. Does not own the Var annotations
// it contains.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef EXPRESSION_SIDE_EFFECT_H
#define EXPRESSION_SIDE_EFFECT_H

#include <analysis/SideEffect.h>

namespace RAnnot {

class ExpressionSideEffect : public AnnotationBase {
public:
  typedef SideEffect::MyVarT MyVarT;
  typedef SideEffect::MyVarSetT MyVarSetT;
  typedef SideEffect::MyIteratorT MyIteratorT;

  explicit ExpressionSideEffect(bool trivial, bool cheap);
  virtual ~ExpressionSideEffect();

  bool is_trivial() const;
  bool is_cheap() const;

  void set_action(bool x);
  bool get_action() const;

  MyVarSetT get_uses() const;
  MyVarSetT get_defs() const;

  void insert_use_sexp(const SEXP sexp);
  void insert_def_sexp(const SEXP sexp);

  void insert_use(const MyVarT & v);
  void insert_def(const MyVarT & v);

  void add(const SideEffect * x);

  MyIteratorT begin_uses() const;
  MyIteratorT end_uses() const;

  MyIteratorT begin_defs() const;
  MyIteratorT end_defs() const;

  // returns true if there is any true, anti, or output dependence between the two
  bool intersects(SideEffect * other) const;

  AnnotationBase * clone();
  static PropertyHndlT handle();

  std::ostream & dump(std::ostream & os) const;

  SideEffect * get_side_effect() const;

private:
  SideEffect * m_side_effect;
};

}

#endif
