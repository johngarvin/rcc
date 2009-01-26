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
typedef ExpressionInfo::MyCallSiteT MyCallSiteT;
typedef ExpressionInfo::const_use_iterator const_use_iterator;
typedef ExpressionInfo::const_def_iterator const_def_iterator;
typedef ExpressionInfo::call_site_iterator call_site_iterator;
typedef ExpressionInfo::const_call_site_iterator const_call_site_iterator;
typedef ExpressionInfo::MyLazyInfoSetT MyLazyInfoSetT;

ExpressionInfo::ExpressionInfo(SEXP cell)
  : m_cell(cell),
    m_uses(), m_defs(),
    m_call_sites(),
    m_strict(false),
    m_trivially_evaluable(false),
    m_lazy_info(is_call(CAR(cell)) ? Rf_length(call_args(CAR(cell))) + 1 : 0) // +1 because indexed from 1
{
}


ExpressionInfo::~ExpressionInfo()
{
}

// set operations
void ExpressionInfo::insert_use(UseVar * const x) {
  assert(x != 0);
  m_uses.push_back(x);
}

void ExpressionInfo::insert_def(DefVar * const x) {
  assert(x != 0);
  m_defs.push_back(x);
}

void ExpressionInfo::insert_call_site(const MyCallSiteT & x) {
  assert(x != 0);
  m_call_sites.push_back(x);
}

// iterators
const_use_iterator ExpressionInfo::begin_uses() const {
  return m_uses.begin();
}

const_use_iterator ExpressionInfo::end_uses() const {
  return m_uses.end();
}

const_def_iterator ExpressionInfo::begin_defs() const {
  return m_defs.begin();
}

const_def_iterator ExpressionInfo::end_defs() const {
  return m_defs.end();
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
SEXP ExpressionInfo::get_cell() const {
  return m_cell;
}

void ExpressionInfo::set_cell(SEXP x) {
  m_cell = x;
}

EagerLazyT ExpressionInfo::get_eager_lazy(int arg) const {
  return m_lazy_info.at(arg);
}

void ExpressionInfo::set_eager_lazy(int arg, EagerLazyT x) {
  m_lazy_info.at(arg) = x;
}


AnnotationBase * ExpressionInfo::clone() {
  return new ExpressionInfo(*this);
}

std::ostream & ExpressionInfo::dump(std::ostream & os) const {
  UseVar * use;
  DefVar * def;
  SEXP cs;
  beginObjDump(os, ExpressionInfo);
  SEXP definition = CAR(m_cell);
  dumpSEXP(os, definition);

  os << "Begin uses:" << std::endl;
  EXPRESSION_FOR_EACH_USE(this, use) {
    use->dump(os);
  }
  os << "End uses" << std::endl;

  os << "Begin defs:" << std::endl;
  EXPRESSION_FOR_EACH_DEF(this, def) {
    def->dump(os);
  }
  os << "End defs" << std::endl;

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
