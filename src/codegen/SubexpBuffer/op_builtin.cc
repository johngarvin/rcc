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

// File: op_builtin.cc
//
// Output a function application where the function is of type
// BUILTINSXP. This includes unary and binary logical and arithmetic
// operators.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>

#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <CodeGen.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

bool is_constant_expr(SEXP s) {
  switch(TYPEOF(s)) {
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
    return true;
  default:
    break;
  }
  return false;
}

Expression SubexpBuffer::op_builtin(SEXP e, SEXP op, string rho, 
				    Protection resultProtection)
{
  string out;
#ifdef USE_OUTPUT_CODEGEN
  Expression op1 = output_to_expression(CodeGen::op_primsxp(op, rho));
#else
  Expression op1 = ParseInfo::global_constants->op_primsxp(op, rho);
#endif
  SEXP args = CDR(e);
  // special case for arithmetic operations
  if (PRIMFUN(op) == (CCODE)do_arith) {

    // R_unary if there's one argument and it's a non-object
    if (args != R_NilValue
	&& CDR(args) == R_NilValue
	&& !Rf_isObject(CAR(args))) { // one argument, non-object

      //----------------------------------------------------------
      // optimization note:
      //   R_unary is safe to call with unprotected arguments
      //   it may duplicate its argument, but duplicate protects its
      //   before allocating memory argument
      //
      // 14 September 2005 - John Mellor-Crummey
      //----------------------------------------------------------
      Expression x = op_exp(args, rho, Unprotected, TRUE);
      out = appl3("R_unary", "R_NilValue", op1.var, x.var, resultProtection);
      del(x);

    // R_binary if two non-object arguments
    } else if (CDDR(args) == R_NilValue && 
	       !Rf_isObject(CAR(args))
	       && !Rf_isObject(CADR(args))) {

      //----------------------------------------------------------
      // optimization note:
      //   R_binary is safe to call with unprotected arguments. it
      //   protects its arguments before allocating a result.
      //  
      //   note: unless evaluation of the second argument is known to 
      //   not allocate memory, the result of evaluating the 
      //   first argument must be protected. in the absence of this
      //   knowledge, we protect it.
      //
      // 14 September 2005 - John Mellor-Crummey
      //----------------------------------------------------------
      Protection xprot = Protected;
      if (is_constant_expr(CAR(args))) {
	xprot = Unprotected;
      }
      Expression x = op_exp(args, rho, xprot, TRUE);
      Expression y = op_exp(CDR(args), rho, Unprotected, TRUE);
      out = appl4("R_binary", "R_NilValue", op1.var, x.var, y.var, 
		  Unprotected);
      int unprotcnt = 0;
      if (!x.del_text.empty()) unprotcnt++;
      if (!y.del_text.empty()) unprotcnt++;
      if (unprotcnt > 0) 
	append_defs("UNPROTECT(" + i_to_s(unprotcnt) + ");\n");
      if (resultProtection == Protected) {
	if (unprotcnt > 0) 
	  append_defs("SAFE_PROTECT(" + out + ");\n");
	else
	  append_defs("PROTECT(" + out + ");\n");
      }
    }
    
  } else {  // common case: call the do_ function
#if USE_OUTPUT_CODEGEN
    Expression args1 = output_to_expression(CodeGen::op_list(args, rho, false, true));
#else
    Expression args1 = op_list(args, rho, false, Protected, true);
#endif
    // FIXME:
    // here: don't we need to check whether args1 is dependent? Also op1?
    out = appl4(get_name(PRIMOFFSET(op)),
		"R_NilValue ",
		op1.var,
		args1.var,
		rho, Unprotected);
    int unprotcnt = 0;
    if (!args1.del_text.empty()) unprotcnt++;
    if (unprotcnt > 0) 
      append_defs("UNPROTECT(" + i_to_s(unprotcnt) + ");\n");
    if (resultProtection == Protected) {
      if (unprotcnt > 0) 
	append_defs("SAFE_PROTECT(" + out + ");\n");
      else
	append_defs("PROTECT(" + out + ");\n");
    }
  }
  string cleanup;
  if (resultProtection == Protected) cleanup = unp(out);

  del(op1);
  return Expression(out,
		    DEPENDENT,
		    1 - PRIMPRINT(op) ? VISIBLE : INVISIBLE, 
		    cleanup);
}
