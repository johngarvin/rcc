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

// File: ResolvedCallByValueInfo.h
//
// Eager/lazy information for each resolved actual argument of a call
// site. For call sites for which ResolvedArgsAnnotationMap has
// resolved arguments.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef RESOLVED_CALL_BY_VALUE_INFO_H
#define RESOLVED_CALL_BY_VALUE_INFO_H

#include <vector>

#include <analysis/AnnotationBase.h>
#include <analysis/EagerLazy.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class ResolvedCallByValueInfo : public AnnotationBase {
public:
  typedef std::vector<EagerLazyT> MyLazyInfoSetT;

  explicit ResolvedCallByValueInfo(int argc);
  virtual ~ResolvedCallByValueInfo();

  /// Not supported.
  virtual AnnotationBase * clone();

  static PropertyHndlT handle();

  MyLazyInfoSetT get_eager_lazy_info() const;

  EagerLazyT get_eager_lazy(int arg) const;
  void set_eager_lazy(int arg, EagerLazyT x);

  virtual std::ostream & dump(std::ostream &) const;

private:
  MyLazyInfoSetT m_eager_lazy;
};

} // end namespace RAnnot

#endif
