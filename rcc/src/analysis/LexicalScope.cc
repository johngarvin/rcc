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

// File: LexicalScope.cc
//
// Represents an R lexical scope. A lexical scope can be pre-defined
// (InternalLexicalScope) (e.g. the scope where R internal names are
// bound) or created by a function definition (FundefLexicalScope). The
// global scope, because it contains code, is represented as a
// FundefLexicalScope.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <analysis/FuncInfo.h>
#include <analysis/AnalysisResults.h>
#include <analysis/SymbolTable.h>
#include <analysis/Utils.h>

#include "LexicalScope.h"

using namespace RAnnot;

//------------------------------------------------------------
// LexicalScope
//------------------------------------------------------------

LexicalScope::LexicalScope() : m_st(new SymbolTable()) {
}

LexicalScope::~LexicalScope() {
  delete m_st;
}

SymbolTable * LexicalScope::get_symbol_table() const {
  return m_st;
}

void LexicalScope::dump(std::ostream & os) const {
  os << get_name();
}

//------------------------------------------------------------
// InternalLexicalScope
//------------------------------------------------------------

InternalLexicalScope::InternalLexicalScope() {
}

InternalLexicalScope * InternalLexicalScope::s_instance = 0;

InternalLexicalScope * InternalLexicalScope::instance() {
  if (s_instance == 0) {
    s_instance = new InternalLexicalScope();
  }
  return s_instance;
}

const std::string InternalLexicalScope::get_name() const {
  return "<Internal scope>";
}

bool InternalLexicalScope::has_children() const {
  // Internal scopes are lexical parents of all user code.
  return true;
}

//------------------------------------------------------------
// FundefLexicalScope
//------------------------------------------------------------

FundefLexicalScope::FundefLexicalScope(SEXP sexp) 
  : m_sexp(sexp)
{
}

SEXP FundefLexicalScope::get_sexp() const {
  return m_sexp;
}

const std::string FundefLexicalScope::get_name() const {
  FuncInfo * fi = getProperty(FuncInfo, get_sexp());
  return var_name(CAR(fi->get_first_name_c()));
}

bool FundefLexicalScope::has_children() const {
  FuncInfo * fi = getProperty(FuncInfo, get_sexp());
  return fi->has_children();
}

//------------------------------------------------------------
// UnboundLexicalScope
//------------------------------------------------------------

UnboundLexicalScope::UnboundLexicalScope() {
}

UnboundLexicalScope * UnboundLexicalScope::s_instance = 0;

UnboundLexicalScope * UnboundLexicalScope::instance() {
  if (s_instance == 0) {
    s_instance = new UnboundLexicalScope();
  }
  return s_instance;
}

const std::string UnboundLexicalScope::get_name() const {
  return "<Unbound>";
}

bool UnboundLexicalScope::has_children() const {
  return false;
}
