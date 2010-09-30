// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: SymbolTableFacade.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "SymbolTableFacade.h"

#include <analysis/AnalysisResults.h>
#include <analysis/VarBinding.h>

using namespace RAnnot;

SymbolTableFacade::SymbolTableFacade() {
}

SymbolTableFacade::~SymbolTableFacade() {
}

VarInfo * SymbolTableFacade::find_entry(const SEXP sexp) const {
  VarInfo * vi;
  Var * var = getProperty(Var, sexp);
  VarBinding * binding = getProperty(VarBinding, sexp);
  if (binding->is_unbound()) {
    SymbolTable * table = UnboundLexicalScope::instance()->get_symbol_table();
    vi = (*table)[var->get_name()];
  } else if (binding->is_single()) {
    SymbolTable * table = (*binding->begin())->get_symbol_table();
    vi = (*table)[var->get_name()];
  } else {
    vi = (*SymbolTable::get_ambiguous_st())[var->get_name()];
  }
  assert(vi != 0);
  return vi;
}

VarInfo * SymbolTableFacade::find_entry(const Var * var) const {
  VarInfo * vi;
  VarBinding * binding = getProperty(VarBinding, var->get_mention_c());
  if (binding->is_unbound()) {
    SymbolTable * table = UnboundLexicalScope::instance()->get_symbol_table();
    vi = (*table)[var->get_name()];
  } else if (binding->is_single()) {
    SymbolTable * table = (*binding->begin())->get_symbol_table();
    vi = (*table)[var->get_name()];
  } else {
    vi = (*SymbolTable::get_ambiguous_st())[var->get_name()];
  }
  assert(vi != 0);
  return vi;
}

SymbolTableFacade * SymbolTableFacade::instance() {
  if (s_instance == 0) {
    s_instance = new SymbolTableFacade();
  }
  return s_instance;
}

SymbolTableFacade * SymbolTableFacade::s_instance = 0;
