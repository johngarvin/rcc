// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
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

// File: FormalArgInfo.h
//
// Annotation for a formal argument of a function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_FORMAL_ARG_INFO_H
#define ANNOTATION_FORMAL_ARG_INFO_H

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// FormalArgInfo
// ---------------------------------------------------------------------------
class FormalArgInfo : public AnnotationBase
{
public:
  explicit FormalArgInfo(SEXP cell);
  virtual ~FormalArgInfo();

  static PropertyHndlT handle();

  // -------------------------------------------------------
  // cloning: not supported
  // -------------------------------------------------------
  virtual FormalArgInfo * clone();

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream & dump(std::ostream & os) const;
  
  bool is_strict() const;
  void set_is_strict(bool x);

  bool has_default() const;
  SEXP get_default() const;

  bool is_named() const;
  SEXP get_name() const;

  SEXP get_sexp() const;

private:
  const SEXP m_sexp;     // TAG(m_sexp) is the name of the argument
  const SEXP m_default;  // default arg if it exists; otherwise 0
  const SEXP m_name;     // SYMSXP: the name of the argument if it exists
  bool m_is_value; // value/promise
  bool m_is_strict; // function always evaluates this argument
};


}

#endif
