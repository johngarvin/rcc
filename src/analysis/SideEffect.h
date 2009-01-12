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

#include <OpenAnalysis/Location/Location.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/AnnotationBase.h>
#include <analysis/FuncInfo.h>
#include <analysis/Var.h>

namespace RAnnot {

class SideEffect : public AnnotationBase {
public:
  typedef OA::OA_ptr<OA::Location>   MyVarT;
  typedef OA::OA_ptr<OA::LocSet>     MyVarSetT;
  typedef OA::LocSet::const_iterator MyIterator;

  explicit SideEffect();
  virtual ~SideEffect();

  void set_trivial(bool x);
  bool is_trivial();
  void set_cheap(bool x);
  bool is_cheap();

  MyVarSetT get_uses();
  MyVarSetT get_defs();

  void insert_mention(const FuncInfo * fi, const Var * v);
  void insert_use(const MyVarT & v);
  void insert_def(const MyVarT & v);

  void add(const SideEffect * x);

  MyIterator begin_uses() const;
  MyIterator end_uses() const;
  MyIterator begin_defs() const;
  MyIterator end_defs() const;

  // returns true if there is any true, anti, or output dependence between the two
  bool intersects(SideEffect * other);

  AnnotationBase * clone();
  static PropertyHndlT handle();

  std::ostream& dump(std::ostream& os) const;

private:
  MyVarSetT m_uses;
  MyVarSetT m_defs;

  bool m_trivial;
  bool m_cheap;
};

}  // end namespace RAnnot

#endif
