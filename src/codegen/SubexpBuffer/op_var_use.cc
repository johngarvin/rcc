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

// File: op_var_use.cc
//
// Output a use of a symbol.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#include <support/StringUtils.h>
#include <support/RccError.h>

#include <CodeGen.h>
#include <CodeGenUtils.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;
using namespace RAnnot;


// file-static forward declarations

typedef enum { PLAIN_VAR, FUNCTION_VAR } LookupType;

static Expression op_use(SubexpBuffer *sb, SEXP cell, string rho,
			 Protection resultProtection,
			 bool fullyEvaluatedResult, LookupType lookup_type);
static Expression op_lookup(SubexpBuffer * sb, string lookup_function,
			    string symbol, string rho,
			    Protection resultProtection,
			    bool fullyEvaluatedResult);
static Expression op_internal(SubexpBuffer * sb, SEXP e, SEXP env_val, string name,
			      string lookup_function, string rho);

// interface functions

Expression SubexpBuffer::op_var_use(SEXP cell, string rho,
				    Protection resultProtection,
				    bool fullyEvaluatedResult)
{
  return op_use(this, cell, rho, resultProtection, fullyEvaluatedResult, PLAIN_VAR);
}

Expression SubexpBuffer::op_fun_use(SEXP cell, string rho,
				    Protection resultProtection,
				    bool fullyEvaluatedResult)
{
  return op_use(this, cell, rho, resultProtection, fullyEvaluatedResult, FUNCTION_VAR);
}


// file-static function definitions

static Expression op_use(SubexpBuffer *sb, SEXP cell, string rho,
			 Protection resultProtection,
			 bool fullyEvaluatedResult, LookupType lookup_type)
{
  SEXP e = CAR(cell);
  string name = var_name(e);
  string lookup_function = (lookup_type == FUNCTION_VAR ? "Rf_findFun" : "Rf_findVar");
  VarBinding * binding = getProperty(VarBinding, cell);

  if (binding->is_single()) {
    if (InternalLexicalScope * scope = dynamic_cast<InternalLexicalScope *>(*(binding->begin()))) {
      // check for global constants
      // binding map in ParseInfo is used for global variables with
      // constant values; it records the constant value of each name.
      if (ParseInfo::binding_exists(name)) {
	return Expression(ParseInfo::get_binding(name), CONST, VISIBLE, "");
      } else {
	SEXP env_val;
	if (lookup_type == FUNCTION_VAR) {
	  env_val = library_value(e);
	} else {
	  env_val = Rf_findVar(e, R_GlobalEnv);
	}
	assert(env_val != R_UnboundValue);
	return op_internal(sb, e, env_val, name, lookup_function, rho);
      }
    } else if (FundefLexicalScope * scope = dynamic_cast<FundefLexicalScope *>(*(binding->begin()))) {
      FuncInfo* fi = getProperty(FuncInfo, scope->get_sexp());
      if (fi->is_arg(e)) {
	// have to produce a lookup here; we don't know whether we need to evalaute a promise
	// TODO: handle this case
	return op_lookup(sb, lookup_function, make_symbol(e), rho,
			 resultProtection, fullyEvaluatedResult);
      } else {
	string location = binding->get_location(e, sb);
	string h = sb->appl1("R_GetVarLocValue", location, Unprotected);
	return Expression(h, DEPENDENT, VISIBLE, "");
      }
    } else if (UnboundLexicalScope * scope = dynamic_cast<UnboundLexicalScope *>(*(binding->begin()))) {
      rcc_error("Attempted to use an unbound variable");
    } else {
      rcc_error("Unknown derived type of LexicalScope found");
    }
  } else {   // more than one possible scope
    return op_lookup(sb, lookup_function, make_symbol(e), rho,
		     resultProtection, fullyEvaluatedResult);
  }
}

/// output a lookup of a name in the given environment
static Expression op_lookup(SubexpBuffer * sb, string lookup_function,
			    string symbol, string rho,
			    Protection resultProtection, bool fullyEvaluatedResult)
{
  string value = sb->appl2(lookup_function, symbol, rho, Unprotected);
  string del_text;
  VisibilityType vis = VISIBLE;
  if (fullyEvaluatedResult) {
    value = sb->appl2("Rf_eval", value, rho, resultProtection);
    vis = CHECK_VISIBLE;
    if (resultProtection == Protected) del_text = unp(value);
  }
  return Expression(value, DEPENDENT, vis, del_text);
}


/// output the value of an internal R name
static Expression op_internal(SubexpBuffer * sb, SEXP e, SEXP env_val, string name,
			      string lookup_function, string rho)
{
  string h;
  SEXP sym_value = SYMVALUE(e);
  if (sym_value == R_UnboundValue) {
    // library function
    string call = emit_call2(lookup_function, make_symbol(e), "R_GlobalEnv");
    if (TYPEOF(env_val) == PROMSXP) {
      call = emit_call2("Rf_eval", call, "R_GlobalEnv");
    }
    h = ParseInfo::global_constants->new_sexp();
    string assign = emit_prot_assign(h, call);
    ParseInfo::global_constants->append_defs(assign);
  } else {
    // builtin/special function; symbol containts ptr to definition 
    if (TYPEOF(sym_value) == PROMSXP) {
      string sv = emit_call1("SYMVALUE", make_symbol(e));
      h = ParseInfo::global_constants->appl2("Rf_eval", sv, "R_GlobalEnv");
    } else {
      h = ParseInfo::global_constants->appl1("SYMVALUE", make_symbol(e), Unprotected);
    }
  }
  ParseInfo::insert_binding(name, h);
  return Expression(h, CONST, VISIBLE, "");
}

