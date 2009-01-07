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

// File: ExpressionInfo.cc
//
// Annotation for expressions.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/DumpMacros.h>

#include <analysis/ExpressionInfoAnnotationMap.h>
#include <analysis/Utils.h>
#include <analysis/Var.h>

#include "ExpressionInfo.h"

namespace RAnnot {

// typedefs for readability
typedef ExpressionInfo::MyVarT MyVarT;
typedef ExpressionInfo::MyCallSiteT MyCallSiteT;
typedef ExpressionInfo::var_iterator var_iterator;
typedef ExpressionInfo::const_var_iterator const_var_iterator;
typedef ExpressionInfo::call_site_iterator call_site_iterator;
typedef ExpressionInfo::const_call_site_iterator const_call_site_iterator;
typedef ExpressionInfo::MyLazyInfoSetT MyLazyInfoSetT;

ExpressionInfo::ExpressionInfo(SEXP sexp)
  : m_sexp(sexp),
    m_strict(false),
    m_trivially_evaluable(false),
    m_lazy_info(is_call(sexp) ? Rf_length(call_args(sexp)) + 1 : 0) // +1 because indexed from 1
{
}


ExpressionInfo::~ExpressionInfo()
{
}

// set operations
void ExpressionInfo::insert_var(const MyVarT & x) {
  m_vars.push_back(x);
}

void ExpressionInfo::insert_call_site(const MyCallSiteT & x) {
  m_call_sites.push_back(x);
}

// iterators
var_iterator ExpressionInfo::begin_vars() {
  return m_vars.begin();
}

const_var_iterator ExpressionInfo::begin_vars() const {
  return m_vars.begin();
}

var_iterator ExpressionInfo::end_vars() {
  return m_vars.end();
}

const_var_iterator ExpressionInfo::end_vars() const {
  return m_vars.end();
}

call_site_iterator ExpressionInfo::begin_call_sites() {
  return m_call_sites.begin();
}
const_call_site_iterator ExpressionInfo::begin_call_sites() const {
  return m_call_sites.begin();
}

call_site_iterator ExpressionInfo::end_call_sites() {
  return m_call_sites.end();
}

const_call_site_iterator ExpressionInfo::end_call_sites() const {
  return m_call_sites.end();
}

// annotations for strictness
bool ExpressionInfo::is_strict() {
  return m_strict;
}

void ExpressionInfo::set_strict(bool x) {
  m_strict = x;
}

bool ExpressionInfo::is_trivially_evaluable() {
  return m_trivially_evaluable;
}

void ExpressionInfo::set_trivially_evaluable(bool x) {
  m_trivially_evaluable = x;
}

// definition of the expression
SEXP ExpressionInfo::get_sexp() const {
  return m_sexp;
}

void ExpressionInfo::set_sexp(SEXP x) {
  m_sexp = x;
}

EagerLazyT ExpressionInfo::get_eager_lazy(int arg) const {
  return m_lazy_info[arg];
}

void ExpressionInfo::set_eager_lazy(int arg, EagerLazyT x) {
  m_lazy_info[arg] = x;
}


AnnotationBase * ExpressionInfo::clone() {
  return new ExpressionInfo(*this);
}

std::ostream & ExpressionInfo::dump(std::ostream & os) const {
  Var * var;
  SEXP cs;
  beginObjDump(os, ExpressionInfo);
  SEXP definition = CAR(m_sexp);
  dumpSEXP(os, definition);

  os << "Begin mentions:" << std::endl;
  EXPRESSION_FOR_EACH_MENTION(this, var) {
    var->dump(os);
  }
  os << "End mentions" << std::endl;

  os << "Begin call sites:" << std::endl;
  EXPRESSION_FOR_EACH_CALL_SITE(this, cs) {
    dumpSEXP(os, CAR(cs));
  }
  os << "End call sites" << std::endl;

  endObjDump(os, ExpressionInfo);
}

PropertyHndlT ExpressionInfo::handle() {
  return ExpressionInfoAnnotationMap::handle();
}

} // end namespace RAnnot
