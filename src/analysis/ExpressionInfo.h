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

  typedef Var *                      MyVar_t;
  typedef std::list<MyVar_t>         MyVarSet_t;
  typedef MyVarSet_t::iterator       var_iterator;
  typedef MyVarSet_t::const_iterator const_var_iterator;

  typedef SEXP                            MyCallSite_t;
  typedef std::list<MyCallSite_t>         MyCallSiteSet_t;
  typedef MyCallSiteSet_t::iterator       call_site_iterator;
  typedef MyCallSiteSet_t::const_iterator const_call_site_iterator;

  // -------------------------------------------------------
  // set operations
  // -------------------------------------------------------
  void insert_var(const MyVar_t & x)
    { mVars.push_back(x); }
  void insert_call_site(const MyCallSite_t & x)
    { mCallSites.push_back(x); }

  // iterators:
  var_iterator begin_vars()
    { return mVars.begin(); }
  const_var_iterator begin_vars() const 
    { return mVars.begin(); }
  var_iterator end_vars()
    { return mVars.end(); }
  const_var_iterator end_vars() const 
    { return mVars.end(); }

  call_site_iterator begin_call_sites()
    { return mCallSites.begin(); }
  const_call_site_iterator begin_call_sites() const 
    { return mCallSites.begin(); }
  call_site_iterator end_call_sites()
    { return mCallSites.end(); }
  const_call_site_iterator end_call_sites() const 
    { return mCallSites.end(); }

  // definition of the expression
  SEXP getDefn() const
    { return mDefn; }
  void setDefn(SEXP x)
    { mDefn = x; }

  static PropertyHndlT handle();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual ExpressionInfo* clone() { return new ExpressionInfo(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  SEXP mDefn;
  MyVarSet_t mVars;            // contents of set not owned
  MyCallSiteSet_t mCallSites;  // contents of set not owned
};

}

#endif
