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

// File: SideEffect.h
//
// Side effects, including uses and defs. Used as an annotation type
// by SideEffectAnnotationMap. Does not own the Var annotations it
// contains.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VAR_SET_H
#define VAR_SET_H

#include <list>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>

#include <analysis/AnnotationBase.h>
#include <analysis/DefVar.h>
#include <analysis/FuncInfo.h>
#include <analysis/UseVar.h>
#include <analysis/VarInfo.h>

namespace RAnnot {

class SideEffect : public AnnotationBase {
public:
  typedef VarInfo *                             MyVarT;
  typedef std::set<MyVarT>                      MyRawVarSetT;
  typedef const MyRawVarSetT &                  MyVarSetT;
  typedef MyRawVarSetT::const_iterator          MyIteratorT;

  explicit SideEffect();
  virtual ~SideEffect();

  void set_trivial(bool x);
  bool is_trivial() const;
  void set_cheap(bool x);
  bool is_cheap() const;

  void set_action(bool x);
  bool get_action() const;

  MyVarSetT get_uses() const;
  MyVarSetT get_defs() const;

  void insert_use_sexp(const FuncInfo * fi, const SEXP sexp);
  void insert_def_sexp(const FuncInfo * fi, const SEXP sexp);

  void insert_use(const MyVarT & v);
  void insert_def(const MyVarT & v);

  void add(const SideEffect * x);

  MyIteratorT begin_uses() const;
  MyIteratorT end_uses() const;
  MyIteratorT begin_defs() const;
  MyIteratorT end_defs() const;

  // returns true if there is any true, anti, or output dependence between the two
  bool intersects(SideEffect * other);

  AnnotationBase * clone();
  static PropertyHndlT handle();

  std::ostream & dump(std::ostream & os) const;

private:
  // whether there is an "action" side effect, such as writing to the screen
  bool m_action;

  MyRawVarSetT m_uses;
  MyRawVarSetT m_defs;

  bool m_trivial;
  bool m_cheap;
};

}  // end namespace RAnnot

#endif
