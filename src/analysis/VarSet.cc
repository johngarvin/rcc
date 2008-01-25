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

VarSet::VarSet() {
}

VarSet::~VarSet() {
}

void VarSet::insert(Var * & v) {
  m_vars.push_back(v);
}

std::list<Var *>::const_iterator VarSet::begin() const {
  return m_vars.begin();
}

std::list<Var *>::const_iterator VarSet::end() const {
  return m_vars.end();
}

PropertyHndlT VarSet::handle() {
  return SideEffectAnnotationMap::handle();
}

std::ostream& VarSet::dump(std::ostream& os) const {
  beginObjDump(os, VarSet);
  os << "Begin mentions:" << std::endl;
  for(std::list<Var *>::const_iterator it = begin(); it != end(); ++it) {
    (*it)->dump(os);
  }
  os << "End mentions" << std::endl;
  endObjDump(os, VarSet);
}

}  // end namespace RAnnot
