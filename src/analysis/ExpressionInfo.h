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
#include <analysis/DefVar.h>
#include <analysis/EagerLazy.h>
#include <analysis/PropertyHndl.h>
#include <analysis/UseVar.h>

#define EXPRESSION_FOR_EACH_USE(ei, use) for(RAnnot::ExpressionInfo::const_use_iterator vi = (ei)->begin_uses();      \
					     (vi != (ei)->end_uses()) && (((use) = *vi) != 0);                        \
					     ++vi)

#define EXPRESSION_FOR_EACH_DEF(ei, def) for(RAnnot::ExpressionInfo::const_def_iterator vi = (ei)->begin_defs();      \
					     (vi != (ei)->end_defs()) && (((def) = *vi) != 0);                        \
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

  typedef UseVar *                  MyUseT;
  typedef DefVar *                  MyDefT;
  typedef std::list<MyUseT>         MyUseSetT;
  typedef std::list<MyDefT>         MyDefSetT;
  typedef MyUseSetT::const_iterator const_use_iterator;
  typedef MyDefSetT::const_iterator const_def_iterator;

  typedef SEXP                           MyCallSiteT;
  typedef std::list<MyCallSiteT>         MyCallSiteSetT;
  typedef MyCallSiteSetT::iterator       call_site_iterator;
  typedef MyCallSiteSetT::const_iterator const_call_site_iterator;

  typedef std::vector<EagerLazyT> MyLazyInfoSetT;

  // set operations
  void insert_use(MyUseT const x);
  void insert_def(MyDefT const x);
  void insert_call_site(const MyCallSiteT & x);
  void insert_eager_lazy(const EagerLazyT x);

  // iterators:

  MyUseSetT::const_iterator begin_uses() const;
  MyUseSetT::const_iterator end_uses() const;

  MyDefSetT::const_iterator begin_defs() const;
  MyDefSetT::const_iterator end_defs() const;

  call_site_iterator begin_call_sites();
  call_site_iterator end_call_sites();
  const_call_site_iterator begin_call_sites() const;
  const_call_site_iterator end_call_sites() const;

  // strictness analysis, etc.
  bool is_strict();
  void set_strict(bool x);

  bool is_trivially_evaluable();
  void set_trivially_evaluable(bool x);

  // definition of the expression
  SEXP get_cell() const;
  void set_cell(SEXP x);

  // indexed from 0
  EagerLazyT get_eager_lazy(int arg) const;
  void set_eager_lazy(int arg, EagerLazyT x);
  MyLazyInfoSetT get_lazy_info() const;

  static PropertyHndlT handle();

  // clone (not implemented because I don't think anyone uses it;
  // implement this if anyone does)
  AnnotationBase * clone();

  // debugging
  virtual std::ostream& dump(std::ostream& os) const;

private:
  SEXP m_cell;                  // CAR(m_cell) is the expression
  // mentions in expression
  // (contents of set not owned)
  MyUseSetT m_uses;
  MyDefSetT m_defs;
  MyCallSiteSetT m_call_sites;  // call sites in expression
                                //   (contents of set not owned)
  bool m_strict;                // whether expression can be evaluated
                                //   ahead of time without causing
                                //   side effects, diverging, or
                                //   throwing an exception
  bool m_trivially_evaluable;   // whether expression is both strict
                                //   and guaranteed to be cheap to
                                //   evaluate
  MyLazyInfoSetT m_lazy_info;   // if this expression is a call site,
				//   for each actual arg whether it can
				//   be evaulated eagerly for
				//   performance or must conservatively
				//   be called lazy. Indexed from 0.
};

}

#endif
