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
#include <analysis/SideEffect.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// FormalArgInfo
// ---------------------------------------------------------------------------
class FormalArgInfo
  : public AnnotationBase
{
public:
  explicit FormalArgInfo(SEXP cell);
  virtual ~FormalArgInfo();

  static PropertyHndlT handle();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual FormalArgInfo * clone() { return new FormalArgInfo(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream & dump(std::ostream & os) const;
  
  bool is_value();
  void set_is_value(bool x);

  bool is_strict();
  void set_is_strict(bool x);

  bool has_default();
  void set_default(SEXP def);
  SEXP get_default();

  bool is_named() const;
  void set_name(const SEXP sym);
  SEXP get_name() const;

  SideEffect * get_pre_debut_side_effect();
  void set_pre_debut_side_effect(SideEffect * x);

  SEXP get_sexp();

private:
  SEXP m_sexp;     // TAG(m_sexp) is the name of the argument
  bool m_is_value; // value/promise
  bool m_is_strict; // function always evaluates this argument
  SEXP m_default;  // default arg if it exists; otherwise 0
  SEXP m_name;     // SYMSXP: the name of the argument if it exists
  SideEffect * m_pre_debut_side_effect;  // names used/defined by pre-debut statements for this formal
};


}

#endif
