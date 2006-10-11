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
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <analysis/FuncInfo.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

#include "LexicalScope.h"

using namespace RAnnot;

InternalLexicalScope::InternalLexicalScope() {
}

const std::string InternalLexicalScope::get_name() const {
  return "<Internal scope>";
}

FundefLexicalScope::FundefLexicalScope(SEXP fundef) : m_fundef(fundef) {
}

SEXP FundefLexicalScope::get_fundef() const {
  return m_fundef;
}

const std::string FundefLexicalScope::get_name() const {
  FuncInfo * fi = getProperty(FuncInfo, get_fundef());
  return var_name(fi->get_first_name());
}
