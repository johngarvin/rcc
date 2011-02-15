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

// File: BasicVar.h
//
// Basic annotation for a variable reference (includes uses and defs)
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef BASIC_ANNOTATION_VAR_H
#define BASIC_ANNOTATION_VAR_H

#include <cassert>
#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/LocalityType.h>
#include <analysis/PropertyHndl.h>
#include <analysis/VarVisitor.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// Var: A variable reference (includes uses and defs)
// ---------------------------------------------------------------------------
class BasicVar
  : public AnnotationBase
{
public:
  enum UseDefT {
    Var_USE,
    Var_DEF
  };
  
  enum MayMustT {
    Var_MAY,
    Var_MUST
  };

public:
  explicit BasicVar(SEXP m_sexp, UseDefT udt, MayMustT mmt, Locality::LocalityType lt);
  virtual ~BasicVar();
  
  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------
  
  // use/def type
  UseDefT get_use_def_type() const;

  // may/must type
  MayMustT get_may_must_type() const;

  // scope type
  Locality::LocalityType get_basic_scope_type() const;

  // Mention (cons cell that contains the name)
  SEXP get_mention_c() const;

  virtual SEXP get_name() const = 0;

  virtual void accept(VarVisitor * v) = 0;

  static PropertyHndlT handle();

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream & dump(std::ostream & os) const;

private:
  const SEXP m_sexp;
  const UseDefT m_use_def_type;
  const MayMustT m_may_must_type;
  Locality::LocalityType m_basic_scope_type;
};

const std::string type_name(const BasicVar::UseDefT x);
const std::string type_name(const BasicVar::MayMustT x);

}

#endif
