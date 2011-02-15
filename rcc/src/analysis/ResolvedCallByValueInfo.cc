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

// File: ResolvedCallByValueInfo.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/DumpMacros.h>

#include <analysis/ResolvedCallByValueInfoAnnotationMap.h>

#include "ResolvedCallByValueInfo.h"

namespace RAnnot {

ResolvedCallByValueInfo::ResolvedCallByValueInfo(int argc)
  : m_eager_lazy(argc)
{
}

ResolvedCallByValueInfo::~ResolvedCallByValueInfo() {
}

AnnotationBase * ResolvedCallByValueInfo::clone() {
  return 0;
}

PropertyHndlT ResolvedCallByValueInfo::handle() {
  return ResolvedCallByValueInfoAnnotationMap::handle();
}

ResolvedCallByValueInfo::MyLazyInfoSetT ResolvedCallByValueInfo::get_eager_lazy_info() const {
  return m_eager_lazy;
}

EagerLazyT ResolvedCallByValueInfo::get_eager_lazy(int arg) const {
  return m_eager_lazy.at(arg);
}

void ResolvedCallByValueInfo::set_eager_lazy(int arg, EagerLazyT x) {
  m_eager_lazy[arg] = x;
}

std::ostream & ResolvedCallByValueInfo::dump(std::ostream & os) const {
  int i;

  beginObjDump(os, ResolvedCallByValueInfo);
  for(i=0; i<m_eager_lazy.size(); i++) {
    os << (m_eager_lazy[i] == EAGER ? "EAGER" : "LAZY");
    if (i < m_eager_lazy.size() - 1) {
      os << ", ";
    }
  }
  os << std::endl;
  endObjDump(os, ResolvedCallByValueInfo);
  return os;
}

} // end namespace RAnnot
