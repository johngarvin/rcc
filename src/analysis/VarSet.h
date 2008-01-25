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

// File: VarSet.h
//
// Set of Vars. Used as an annotation type by SideEffectAnnotationMap.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VAR_SET_H
#define VAR_SET_H

#include <list>

#include <analysis/AnnotationBase.h>
#include <analysis/Var.h>

namespace RAnnot {

class VarSet : public AnnotationBase {
public:
  VarSet();
  virtual ~VarSet();

  void insert(Var * & v);
  std::list<Var *>::const_iterator begin() const;
  std::list<Var *>::const_iterator end() const;

  static PropertyHndlT handle();

  std::ostream& dump(std::ostream& os) const;

private:
  std::list<Var *> m_vars;
};

}  // end namespace RAnnot

#endif
