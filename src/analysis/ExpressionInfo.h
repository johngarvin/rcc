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

#include <list>
#include <vector>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <include/R/R_RInternals.h>

#include <analysis/AnnotationBase.h>
#include <analysis/EagerLazy.h>
#include <analysis/PropertyHndl.h>

#define EXPRESSION_FOR_EACH_MENTION(ei, var) for(RAnnot::ExpressionInfo::const_var_iterator vi = (ei)->begin_vars();  \
						 (vi != (ei)->end_vars()) && (((var) = *vi) != 0);                    \
						 ++vi)

#define EXPRESSION_FOR_EACH_CALL_SITE(ei, cs) for(RAnnot::ExpressionInfo::const_call_site_iterator csi = (ei)->begin_call_sites();  \
						    (csi != (ei)->end_call_sites()) && (((cs) = *csi) != 0);                        \
						  ++csi)

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
  explicit ExpressionInfo(SEXP cell);
  virtual ~ExpressionInfo();

  typedef Var *                     MyVarT;
  typedef std::list<MyVarT>         MyVarSetT;
  typedef MyVarSetT::iterator       var_iterator;
  typedef MyVarSetT::const_iterator const_var_iterator;

  typedef SEXP                           MyCallSiteT;
  typedef std::list<MyCallSiteT>         MyCallSiteSetT;
  typedef MyCallSiteSetT::iterator       call_site_iterator;
  typedef MyCallSiteSetT::const_iterator const_call_site_iterator;

  typedef std::vector<EagerLazyT> MyLazyInfoSetT;

  // set operations
  void insert_var(const MyVarT & x);
  void insert_call_site(const MyCallSiteT & x);
  void insert_eager_lazy(const EagerLazyT x);

  // iterators:
  var_iterator begin_vars();
  const_var_iterator begin_vars() const;
  var_iterator end_vars();
  const_var_iterator end_vars() const;

  call_site_iterator begin_call_sites();
  const_call_site_iterator begin_call_sites() const;
  call_site_iterator end_call_sites();
  const_call_site_iterator end_call_sites() const;

  // strictness analysis, etc.
  bool is_strict();
  void set_strict(bool x);

  bool is_trivially_evaluable();
  void set_trivially_evaluable(bool x);

  // definition of the expression
  SEXP get_cell() const;
  void set_cell(SEXP x);

  EagerLazyT get_eager_lazy(int arg) const;
  void set_eager_lazy(int arg, EagerLazyT x);

  static PropertyHndlT handle();

  // clone (not implemented because I don't think anyone uses it;
  // implement this if anyone does)
  AnnotationBase * clone();

  // debugging
  virtual std::ostream& dump(std::ostream& os) const;

private:
  SEXP m_cell;                  // CAR(m_cell) is the expression
  MyVarSetT m_vars;             // mentions in expression
                                // (contents of set not owned)
  MyCallSiteSetT m_call_sites;  // call sites in expression
                                // (contents of set not owned)
  bool m_strict;                // whether expression can be evaluated
                                //   ahead of time without causing
                                //   side effects, diverging, or
                                //   throwing an exception
  bool m_trivially_evaluable;   // whether expression is both strict
                                //   and guaranteed to be cheap to
                                //   evaluate
  MyLazyInfoSetT m_lazy_info;   // if this expression is a call site,
				// for each actual arg whether it can
				// be evaulated eagerly for
				// performance or must conservatively
				// be called lazy. Indexed from 1.
};

}

#endif
