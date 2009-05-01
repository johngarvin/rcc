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

// File: op_lang.cc
//
// Output an expression of type LANGSXP (a function/operator application).
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <analysis/AnalysisResults.h>
#include <analysis/HandleInterface.h>
#include <analysis/OACallGraphAnnotation.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/Settings.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#if 0
  // we are using OpenAnalysis call graphs instead
  #include <analysis/call-graph/RccCallGraphAnnotation.h>
  #include <analysis/call-graph/CallGraphNode.h>
  #include <analysis/call-graph/LibraryCallGraphNode.h>
  #include <analysis/call-graph/FundefCallGraphNode.h>
  #include <analysis/call-graph/UnknownValueCallGraphNode.h>
#endif

#include <support/StringUtils.h>
#include <support/RccError.h>

#include <CodeGen.h>
#include <Metrics.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;
using namespace RAnnot;
using namespace HandleInterface;

static Expression op_internal_call(SubexpBuffer * sb, const SEXP op, SEXP cell,
				   string rho, Protection resultProtection, ResultStatus resultStatus);

static bool debug = false;

Expression SubexpBuffer::op_lang(SEXP cell, string rho, 
				 Protection resultProtection,
				 ResultStatus resultStatus)
{
  SEXP e = CAR(cell);
  if (TYPEOF(call_lhs(e)) == SYMSXP) {
    // check for SPECIALSXP type
    // the value is conveniently stored in the symbol, so we can just grab it
    // redefinition of specials is forbidden, so no need to check call graph
    if (is_library(call_lhs(e)) && TYPEOF(library_value(call_lhs(e))) == SPECIALSXP) {
      return op_special(e, library_value(call_lhs(e)), rho, resultProtection, resultStatus);
    }

    if (Settings::get_instance()->get_call_graph()) {
      // see if symbol is in call graph
      OACallGraphAnnotation * cga = getProperty(OACallGraphAnnotation, e);
      if (cga == 0) {
	// not in call graph; call to internal procedure
	if (is_library(call_lhs(e))) {
	  return op_internal_call(this, library_value(call_lhs(e)), cell, rho, resultProtection, resultStatus);
	} else {
	  rcc_error("Unexpected procedure symbol in neither call graph nor R library");
	}
      } else {
	OA::ProcHandle ph = cga->get_singleton_if_exists();
	if (ph != OA::ProcHandle(0)) {  // singleton exists
	  FuncInfo * fi = getProperty(FuncInfo, make_sexp(ph));
	  Expression closure_exp = Expression(fi->get_closure(), CONST, INVISIBLE, "");
	  Metrics::get_instance()->inc_user_calls();
	  // check for eager assertion
	  if (CDR(call_args(e)) != R_NilValue) {
	    SEXP second_arg = CADR(call_args(e));
	    if (is_rcc_assert_exp(e) && is_var(second_arg) && var_name(second_arg) == "eager.call") {
	      // redo call graph lookup with the real name
	      cga = getProperty(OACallGraphAnnotation, CAR(call_args(e)));
	      ph = cga->get_singleton_if_exists();
	      fi = getProperty(FuncInfo, make_sexp(ph));
	      // TODO: deal with assertions that aren't user-defined symbols
	      // use the real call: pass the cell containing the first argument of eager.call
	      return op_clos_app(closure_exp, call_args(e), rho, resultProtection,
				 EAGER);
	    }
	  }
	  
	  return op_clos_app(closure_exp, cell, rho, resultProtection);
	} else {
	  Metrics::get_instance()->inc_unknown_symbol_calls();
	  Expression func = op_fun_use(e, rho);
	  return op_clos_app(func, cell, rho, resultProtection);
	}
      }
    } else {
      // no call graph
      if (is_library(call_lhs(e))) {
	return op_internal_call(this, library_value(call_lhs(e)), cell, rho, resultProtection, resultStatus);
      } else {
	Metrics::get_instance()->inc_unknown_symbol_calls();
	Expression func = op_fun_use(e, rho);
	return op_clos_app(func, cell, rho, resultProtection);
      }
    }

#if 0

old code with home-grown call graph

    // see if call graph supplies a single definition
    RccCallGraphAnnotation * ann = getProperty(RccCallGraphAnnotation, e);
    const CallGraphNode * node = ann->get_singleton_if_exists();
    if (node) {
      // node is Fundef, Library, or UnknownValue node
      if (const FundefCallGraphNode * cs = dynamic_cast<const FundefCallGraphNode *>(node)) {
	FuncInfo * fi = getProperty(FuncInfo, cs->get_sexp());
	Expression closure_exp = Expression(fi->get_closure(), CONST, INVISIBLE, "");
	return op_clos_app(closure_exp, call_args(e), rho, resultProtection);
      } else if (const LibraryCallGraphNode * lib = dynamic_cast<const LibraryCallGraphNode *>(node)) {
	return op_internal_call(this, lib->get_value(), cell, rho, resultProtection, resultStatus);
      } else if (const UnknownValueCallGraphNode * uv = dynamic_cast<const UnknownValueCallGraphNode *>(node)) {
	Expression func = op_fun_use(e, rho);
	return op_clos_app(func, call_args(e), rho, resultProtection);
      } else {
	rcc_error("Internal error: in call graph, didn't find expected Fundef, Library, or UnknownValue node");
      }
    } else { // more than one possible function value; let op_fun_use decide
      Expression func = op_fun_use(e, rho);
      return op_clos_app(func, call_args(e), rho, resultProtection);
    }

#endif
  } else {  // left side is not a symbol
    Metrics::get_instance()->inc_non_symbol_calls();
    
    // generate closure and application
    Expression op1;
    op1 = op_exp(e, rho, Unprotected);  // evaluate LHS
    return op_clos_app(op1, cell, rho, resultProtection);
    // eval.c: 395
  }
}

/// Output a call to a library or builtin bound in the R environment.
static Expression op_internal_call(SubexpBuffer * sb, const SEXP op, SEXP cell,
				   string rho, Protection resultProtection, ResultStatus resultStatus)
{
  SEXP e = CAR(cell);
  Expression ret_val;
  if (TYPEOF(op) == CLOSXP) {
    // e.g. dnorm takes this path
    // TODO: detect closures that are wrappers around internal calls
    // For internals, find call-by-value status like this:
    //     EagerLazyT func_eager_lazy = (R_FunTab[prim->u.primsxp.offset].eval) % 10 ? EAGER : LAZY;

    Metrics::get_instance()->inc_library_calls();
    Expression func = sb->op_fun_use(e, rho, resultProtection, false);
    return sb->op_clos_app(func, cell, rho, resultProtection);
  } else if (TYPEOF(op) == BUILTINSXP) {
    return sb->op_builtin(e, op, rho, resultProtection);
  } else {
    rcc_error("Internal error: LANGSXP encountered non-function op");
    return Expression::bogus_exp;
  }
}
