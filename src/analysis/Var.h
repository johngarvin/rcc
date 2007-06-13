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

// File: Var.h
//
// Annotation for a variable reference (includes uses and defs)
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_VAR_H
#define ANNOTATION_VAR_H

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
class Var
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
  Var();
  virtual ~Var();
  
  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------
  
  // use/def type
  UseDefT getUseDefType() const
    { return mUseDefType; }

  // may/must type
  MayMustT getMayMustType() const 
    { return mMayMustType; }
  void setMayMustType(MayMustT x)
    { mMayMustType = x; }

  // scope type
  LocalityType getScopeType() const
    { return mScopeType; }
  void setScopeType(LocalityType x)
    { mScopeType = x; }

  // Mention (cons cell that contains the name)
  SEXP getMention_c() const
    { return mSEXP; }
  void setMention_c(SEXP x)
    { mSEXP = x; }

  bool is_first_on_some_path() const
    { return m_first_on_some_path; }
  void set_first_on_some_path(bool x)
    { m_first_on_some_path = x; }

  virtual SEXP getName() const = 0;

  static PropertyHndlT handle();

  virtual void accept(VarVisitor * v) = 0;

#if 0
  // This was an attempt to implement a generic visitor pattern
  // parametrized by the return value. Didn't quite work for a variety
  // of reasons.

  /// generic method for visitor pattern; allows visitors of any
  /// return type
  template<class R> R accept(VarVisitor<R> * v);

  /// general version of accept to be implemented by derived classes
  virtual void * v_accept(VarVisitor<void *>* v) = 0;
#endif

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  //virtual Var* clone() { return new Var(*this); }

  // -------------------------------------------------------
  // code generation
  // -------------------------------------------------------
  //  generating a handle that can be used for accessing the value:
  //    if binding is unknown
  //      find the scope in which the var will be bound
  //      update the value in the proper scope
  //    if binding is condtional
  //      update full/empty bit
  
  //   genCode - phi: generate a handle that can be used to access value
  //     if all reaching defs do not have the same loc binding
  //       generate new handle bound to proper location
  //     if all reaching defs have same loc binding, do nothing
  void genCodeInit();
  void genCodeRHS();
  void genCodeLHS();

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

protected:
  SEXP mSEXP;
  UseDefT mUseDefType;
  MayMustT mMayMustType;
  LocalityType mScopeType;
  bool m_first_on_some_path;
};

const std::string typeName(const Var::UseDefT x);
const std::string typeName(const Var::MayMustT x);

}

#endif
