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

// File: op_var_def.cc
//
// Output an assigmment of a symbol with the given right side.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisResults.h>
#include <analysis/DefVar.h>
#include <analysis/Utils.h>
#include <analysis/Var.h>
#include <analysis/VarBinding.h>

#include <CodeGen.h>
#include <CodeGenUtils.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;
using namespace RAnnot;

Expression SubexpBuffer::op_var_def(SEXP cell, string rhs, string rho) {
  string symbol = make_symbol(CAR(cell));
  VarBinding * binding = getProperty(VarBinding, cell);
  if (binding->is_single()) {
    string location = binding->get_location(CAR(cell), this);
    assert(!location.empty());
    Var * var = getProperty(Var, cell);
    assert(var->getUseDefType() == Var::Var_DEF);
    FundefLexicalScope * scope = dynamic_cast<FundefLexicalScope *>(*(binding->begin()));
    assert(scope != 0);
    FuncInfo * fi = getProperty(FuncInfo, scope->get_sexp());
    if (var->is_first_on_some_path() && !fi->is_arg(CAR(cell))) {
      // first mention of a local var (not a formal), so emit defineVar
      append_defs(emit_assign(location, emit_call3("defineVarReturnLoc", symbol, rhs, rho)));
      return Expression(rhs, DEPENDENT, INVISIBLE, "");
    } else {
      // name has been defined previously, either as a formal or by assignment
      append_defs(emit_call2("R_SetVarLocValue", location, rhs) + ";\n");
      return Expression(rhs, DEPENDENT, INVISIBLE, "");
    }
  } else {   // no unique location; emit lookup
    append_defs(emit_call3("defineVar", symbol, rhs, rho) + ";\n");
    return Expression(rhs, DEPENDENT, INVISIBLE, "");
  }
}
