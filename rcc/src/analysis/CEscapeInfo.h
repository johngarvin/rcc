// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: CEscapeInfo.h
//
// Annotation for information coming from closure escape analysis:
// whether the closure of a function may outlive its caller. Attached
// to fundefs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_C_ESCAPE_INFO_H
#define ANNOTATION_C_ESCAPE_INFO_H

#include <analysis/AnnotationBase.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class CEscapeInfo : public AnnotationBase {
public:
  explicit CEscapeInfo(bool x);
  virtual ~CEscapeInfo();

  bool may_escape();

  void set_may_escape(bool x);

  AnnotationBase * clone();
  std::ostream & dump(std::ostream &) const;

  static PropertyHndlT handle();
private:
  bool m_may_escape;
};

} // end namespace RAnnot

#endif
