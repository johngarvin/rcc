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

// File: ResolvedArgs.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "ResolvedArgs.h"

#include <analysis/ResolvedArgsAnnotationMap.h>

namespace RAnnot {

  ResolvedArgs::ResolvedArgs(SEXP given, SEXP formals)
    : m_given(given),
      m_lazy_info(Rf_length(formals))
  {
  }

  SEXP ResolvedArgs::get_args() {
    return m_given;
  }

  EagerLazyT ResolvedArgs::get_eager_lazy(int arg) const {
    return m_lazy_info.at(arg);
  }
  
  void ResolvedArgs::set_eager_lazy(int arg, EagerLazyT x) {
    m_lazy_info.at(arg) = x;
  }

  std::vector<EagerLazyT> ResolvedArgs::get_lazy_info() const {
    return m_lazy_info;
  }

  AnnotationBase * ResolvedArgs::clone() {
    return 0;
  }

  std::ostream & ResolvedArgs::dump(std::ostream & stream) const {
    Rf_PrintValue(m_given);
  }

  PropertyHndlT ResolvedArgs::handle() {
    return ResolvedArgsAnnotationMap::handle();
  }

}
