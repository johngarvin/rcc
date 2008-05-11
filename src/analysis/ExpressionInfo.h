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

// File: ExpressionInfo.h
//
// Annotation for expressions.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef ANNOTATION_EXPRESSION_INFO_H
#define ANNOTATION_EXPRESSION_INFO_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

// forward declarations

class Var;

//****************************************************************************
// Annotations: Expressions
//****************************************************************************

// ---------------------------------------------------------------------------
// Expression: Abstract base class for expressions
// ---------------------------------------------------------------------------
class ExpressionInfo
  : public AnnotationBase
{
public:
  ExpressionInfo();
  virtual ~ExpressionInfo();

  typedef Var *                     MyVarT;
  typedef std::list<MyVarT>         MyVarSetT;
  typedef MyVarSetT::iterator       var_iterator;
  typedef MyVarSetT::const_iterator const_var_iterator;

  typedef SEXP                           MyCallSiteT;
  typedef std::list<MyCallSiteT>         MyCallSiteSetT;
  typedef MyCallSiteSetT::iterator       call_site_iterator;
  typedef MyCallSiteSetT::const_iterator const_call_site_iterator;

  // set operations
  void insert_var(const MyVarT & x);
  void insert_call_site(const MyCallSiteT & x);

  // iterators:
  var_iterator begin_vars();
  const_var_iterator begin_vars() const;
  var_iterator end_vars();
  const_var_iterator end_vars() const;

  call_site_iterator begin_call_sites();
  const_call_site_iterator begin_call_sites() const;
  call_site_iterator end_call_sites();
  const_call_site_iterator end_call_sites() const;

  // annotations for strictness
  bool is_strict();

  // definition of the expression
  SEXP get_sexp() const;
  void setDefn(SEXP x);

  static PropertyHndlT handle();

  // clone (not implemented; implement this if anyone uses it)
  AnnotationBase * clone();

  // debugging
  virtual std::ostream& dump(std::ostream& os) const;

private:
  SEXP m_sexp;
  MyVarSetT m_vars;             // contents of set not owned
  MyCallSiteSetT m_call_sites;  // contents of set not owned
};

}

#endif
