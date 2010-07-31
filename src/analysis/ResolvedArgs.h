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

// File: ResolvedArgs.h
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef RESOLVED_ARGS_H
#define RESOLVED_ARGS_H

#include <vector>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/EagerLazy.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class ResolvedArgs : public AnnotationBase {
public:
  explicit ResolvedArgs(SEXP given, SEXP formals);

  typedef std::vector<EagerLazyT> MyLazyInfoSetT;

  SEXP get_args();
  SEXP given_from_resolved(SEXP cell);
  SEXP resolved_from_given(SEXP cell);

  // indexed from 0
  EagerLazyT get_eager_lazy(int arg) const;
  void set_eager_lazy(int arg, EagerLazyT x);
  MyLazyInfoSetT get_lazy_info() const;

  AnnotationBase * clone();
  std::ostream & dump(std::ostream & stream) const;
  static PropertyHndlT handle();

private:
  SEXP m_given;
  SEXP m_resolved;
  MyLazyInfoSetT m_lazy_info;
};

} // namespace RAnnot

#endif
