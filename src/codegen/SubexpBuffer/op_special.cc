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

// File: op_special.cc
//
// Output an application where the left side is of type SPECIALSXP.
// This includes assignments, control flow operators, etc.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Settings.h>
#include <analysis/OEscapeInfo.h>
#include <analysis/OEscapeInfoAnnotationMap.h>
#include <analysis/Utils.h>

#include <support/StringUtils.h>

#include <CodeGen.h>
#include <CodeGenUtils.h>
#include <GetName.h>
#include <Metrics.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;
using RAnnot::OEscapeInfo;

// e                 the expression to be translated. It is an application; its CAR is 'op'.
// op                the operation. On entry we know its R type is SPECIALSXP.
// rho               a string representing the environment
// resultProtection  whether the generated expression is protected
// resultStatus      whether the result is needed by later computation
Expression SubexpBuffer::op_special(SEXP cell, SEXP op, string rho,
			Protection resultProtection,
			ResultStatus resultStatus)
{
  SEXP e = CAR(cell);
  string out;
  if (PRIMFUN(op) == (CCODE)do_set) {
    return op_set(cell, op, rho, resultProtection);
  } else if (PRIMFUN(op) == (CCODE)do_internal) {
    // ".Internal" call
    SEXP internal_call = CADR(e);
    SEXP fun = CAR(internal_call);
    Expression args;
    if (TYPEOF(INTERNAL(fun)) == BUILTINSXP) {
#ifdef USE_OUTPUT_CODEGEN
      args = output_to_expression(CodeGen::op_list(CDR(internal_call), rho, false, true));
#else
      args = op_list(CDR(internal_call), rho, false, Protected, true);
#endif
    } else {
      args = op_exp(internal_call, rho);
    }
#ifdef USE_OUTPUT_CODEGEN
    Expression func = output_to_expression(CodeGen::op_primsxp(INTERNAL(fun), rho));
#else
    Expression func = ParseInfo::global_constants->op_primsxp(INTERNAL(fun), rho);
#endif
    out = appl4(get_name(PRIMOFFSET(INTERNAL(fun))), "op_special: do_internal: " + to_string(e),
		"R_NilValue", func.var, args.var, rho);
    del(func);
    del(args);
    return Expression(out, DEPENDENT, INVISIBLE, unp(out));
  } else if (PRIMFUN(op) == (CCODE)do_function) {
    return op_fundef(e, rho, resultProtection);
  } else if (PRIMFUN(op) == (CCODE)do_begin) {
    return op_begin(CDR(e), rho, resultProtection, resultStatus);
  } else if (PRIMFUN(op) == (CCODE)do_if) {
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_if(e, rho));
#else
    return op_if(e, rho, resultStatus);
#endif
  } else if (PRIMFUN(op) == (CCODE)do_for) {
    if (is_for_colon(e) && Settings::get_instance()->get_for_loop_range_deforestation()) {
      return op_for_colon(e, rho, resultStatus);
    } else {
      return op_for(e, rho, resultStatus);
    }
    //
    // } else if (PRIMFUN(op) == (CCODE)do_while) {
    //   return op_while(e, rho);
    //
  } else if (PRIMFUN(op) == (CCODE)do_break) {
    return op_break(CAR(e), rho);
  } else if (PRIMFUN(op) == (CCODE)do_return) {
    return op_return(cell, rho);
  } else if (PRIMFUN(op) == (CCODE)do_subset3) {   // field access, e.g.  foo$bar
    return op_struct_field(e, op, rho, resultProtection);
  } else {
    // default case for specials: call the (call, op, args, rho) fn
    Metrics::get_instance()->inc_special_calls();
#ifdef USE_OUTPUT_CODEGEN
    Expression op1 = output_to_expression(CodeGen::op_primsxp(op, rho));
    Expression args1 = output_to_expression(CodeGen::op_list(CDR(e), rho, true, true));
#else
    Expression op1 = ParseInfo::global_constants->op_primsxp(op, rho);
    Expression args1 = op_list(CDR(e), rho, true, Protected, true);
#endif
    string call_str = appl2("lcons", "", op1.var, args1.var);
    Expression call = Expression(call_str, CONST, VISIBLE, unp(call_str));
    bool may_escape = getProperty(OEscapeInfo, cell)->may_escape();
    string fallback = "INVALID";
    if (may_escape) {
      fallback = new_var_unp();
      append_decls("Rboolean " + fallback + ";\n");
      append_defs(emit_assign(fallback, "getFallbackAlloc()"));
      append_defs(emit_call1("setFallbackAlloc","TRUE") + ";\n");
    }
    out = appl4(get_name(PRIMOFFSET(op)),
		"op_special default: " + to_string(e),
		call.var,
		op1.var,
		args1.var,
		rho, resultProtection);
    if (may_escape) {
      append_defs(emit_call1("setFallbackAlloc", fallback) + ";\n");
    }
    string cleanup;	
    if (resultProtection == Protected) cleanup = unp(out);
    del(call);
    del(op1);
    del(args1);
    return Expression(out, DEPENDENT, 
		      1 - PRIMPRINT(op) ? VISIBLE : INVISIBLE, 
		      cleanup);
  }
}
