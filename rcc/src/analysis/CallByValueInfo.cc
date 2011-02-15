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

// File: CallByValueInfo.cc
//
// Eager/lazy information for actual arguments.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/DumpMacros.h>

#include <analysis/CallByValueInfoAnnotationMap.h>

#include "CallByValueInfo.h"

namespace RAnnot {

CallByValueInfo::CallByValueInfo(int argc)
  : m_eager_lazy(argc)
{
}

CallByValueInfo::~CallByValueInfo() {
}

AnnotationBase * CallByValueInfo::clone() {
  return 0;
}

PropertyHndlT CallByValueInfo::handle() {
  return CallByValueInfoAnnotationMap::handle();
}

CallByValueInfo::MyLazyInfoSetT CallByValueInfo::get_eager_lazy_info() const {
  return m_eager_lazy;
}

EagerLazyT CallByValueInfo::get_eager_lazy(int arg) const {
  return m_eager_lazy.at(arg);
}

void CallByValueInfo::set_eager_lazy(int arg, EagerLazyT x) {
  m_eager_lazy[arg] = x;
}

std::ostream & CallByValueInfo::dump(std::ostream & os) const {
  int i;

  beginObjDump(os, CallByValueInfo);
  for(i=0; i<m_eager_lazy.size(); i++) {
    os << (m_eager_lazy[i] == EAGER ? "EAGER" : "LAZY");
    if (i < m_eager_lazy.size() - 1) {
      os << ", ";
    }
  }
  os << std::endl;
  endObjDump(os, CallByValueInfo);
  return os;
}

} // end namespace RAnnot
