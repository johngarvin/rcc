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

// File: op_clos_app.cc
//
// Output an application of a closure to arguments. Assumes the
// function has already been produced.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/StringUtils.h>
#include <Visibility.h>
#include <CodeGen.h>

using namespace std;

static Expression op_promise_args(SubexpBuffer * sb, string args1var, SEXP args,
				  int * unprotcnt, string rho);

/// Output an application of a closure to actual arguments.
Expression SubexpBuffer::op_clos_app(Expression op1, SEXP args,
				     string rho,
				     Protection resultProtection)
{
  int unprotcnt = 0;

  // Unlike most R internal functions, applyClosure actually uses its
  // 'call' argument, so we can't just make it R_NilValue.

#ifdef USE_OUTPUT_CODEGEN
  Expression args1 = output_to_expression(CodeGen::op_list(args, rho, true));
#else
  Expression args1 = op_list(args, rho, true, Protected); // true: output literal list
#endif
  string call_str = appl2("lcons", op1.var, args1.var);
  unprotcnt++;  // call_str
  Expression arglist = op_promise_args(this, args1.var, args, &unprotcnt, rho);
  string out = appl5("applyClosure ",
		     call_str,
		     op1.var,
		     arglist.var,
		     rho,
		     "R_NilValue", Unprotected);
  if (!op1.del_text.empty()) unprotcnt++;
  if (!args1.del_text.empty()) unprotcnt++;
  if (unprotcnt > 0)
    append_defs("UNPROTECT(" + i_to_s(unprotcnt) + ");\n");
  string cleanup;
  if (resultProtection == Protected) {
    if (unprotcnt > 0)
      append_defs("SAFE_PROTECT(" + out + ");\n");
    else {
      append_defs("PROTECT(" + out + ");\n");
    }
    cleanup = unp(out);
  }
  return Expression(out, DEPENDENT, CHECK_VISIBLE, cleanup);
}

/// Output the actual argument list as a list of promises to be passed to applyClosure
static Expression op_promise_args(SubexpBuffer * sb, string args1var, SEXP args,
				  int * unprotcnt, string rho) {
  string arglist;
  string cleanup;
  if (args == R_NilValue) {
    arglist = "R_NilValue";
  } else {
    // for now, try compiling all code as call-by-value (CBV).
    Expression arg_value = sb->op_list(args, rho, false, Protected);  // false: output as non-literal
    if (!arg_value.del_text.empty()) (*unprotcnt)++;
    // could be applyClosure already performs the promiseArgs call
#if 0
    arglist = sb->appl2("rcc_promise_args", arg_value.var, rho);
    (*unprotcnt)++;
#else
    arglist = arg_value.var;
#endif
  }
  return Expression(arglist, DEPENDENT, INVISIBLE, "");
}
