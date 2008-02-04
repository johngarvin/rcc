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

#include <analysis/AnnotationBase.h>
#include <analysis/Var.h>

namespace RAnnot {

class SideEffect : public AnnotationBase {
public:
  typedef Var *             MyVarT;
  typedef std::list<MyVarT> MyVarSetT;

  SideEffect();
  virtual ~SideEffect();

  void insert_use(const MyVarT & v);
  void insert_def(const MyVarT & v);
  MyVarSetT::const_iterator begin_uses() const;
  MyVarSetT::const_iterator end_uses() const;
  MyVarSetT::const_iterator begin_defs() const;
  MyVarSetT::const_iterator end_defs() const;

  static PropertyHndlT handle();

  std::ostream& dump(std::ostream& os) const;

private:
  MyVarSetT m_uses;
  MyVarSetT m_defs;
};

}  // end namespace RAnnot

#endif
