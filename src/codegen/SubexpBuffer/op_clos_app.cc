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
#include <analysis/ExpressionInfo.h>
#include <analysis/Utils.h>

#include <support/StringUtils.h>

#include <Visibility.h>
#include <CodeGenUtils.h>
#include <CodeGen.h>

using namespace std;

// op_arglist:
//   for each arg a
//     if a has a value assertion
//       then op_exp to produce v
//       else create promise to produce v
//     add v to cons list
// Ack! No promiseArgs!

static Expression op_arglist(SubexpBuffer * sb,
			     const SEXP cell,
			     int * const unprotcnt,
			     string & laziness_string,
			     const string rho);
static Expression op_arglist_rec(SubexpBuffer * const sb,
				 const SEXP args,
				 const RAnnot::ExpressionInfo * const ei, 
				 const int n,
				 int * const unprotcnt,
				 string & laziness_string,
				 const string rho);
static Expression op_promise_args(SubexpBuffer * sb,
				  string args1var,
				  SEXP args,
				  int * unprotcnt,
				  string rho);

/// Output an application of a closure to actual arguments.
Expression SubexpBuffer::op_clos_app(Expression op1, SEXP cell,
				     string rho,
				     Protection resultProtection,
				     EagerLazyT laziness /* = LAZY */)
{
  SEXP e = CAR(cell);
  SEXP args = call_args(e);
  int unprotcnt = 0;
  string laziness_string;  // string of E's and L's, one for each arg, saying whether it's eager or lazy

  // Unlike most R internal functions, applyClosure actually uses its
  // 'call' argument, so we can't just make it R_NilValue.

  Expression args1;
  if (laziness == EAGER) {
#ifdef USE_OUTPUT_CODEGEN
    args1 = output_to_expression(CodeGen::op_list(args, rho, false));  // false: output compiled list
#else
    args1 = op_list(args, rho, false, Protected);                      // false: output compiled list
#endif
    laziness_string = "eager";
  } else {
    args1 = op_arglist(this, cell, &unprotcnt, laziness_string, rho);
  }
  string call_str = appl2("lcons", "", op1.var, args1.var);
  unprotcnt++;  // call_str
  string out = appl5("applyClosure ",
		     "op_clos_app: " + to_string(e) + " " + laziness_string,
		     call_str,
		     op1.var,
		     args1.var,
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

// iterative:
// op_arglist():
//   list = nil
//   for each nth arg a from last to first:
//     if arg is call-by-value:
//       arg_exp = op_exp(arg)
//     else [call-by-need]:
//       arg_exp = op_exp(first, literal)
//       arg_exp = appl2("mkPROMISE", arg_exp.var, rho)
//     end if
//     list = appl2("cons", arg_exp.var, list)
//   end for
//   return appl1("PROTECT", list.var)

// recursive:
// op_arglist(info, list, n):    -- n is the index of the current argument
//   if list = nil:
//     return nil
//   head = CAR(list)
//   tail = CDR(list)
//   tail_exp = op_arglist(tail, n + 1)
//   if info[n] is eager:
//     head_exp = op_exp(head)
//   else: [lazy]
//     head_exp = op_exp(head, literal)
//     head_exp = appl2("mkPROMISE", head_exp, rho)
//   end if
//   return appl2("cons", head_exp, tail_exp)

static Expression op_arglist(SubexpBuffer * const sb, const SEXP cell, int * const unprotcnt, string & laziness_string, string rho) {
  SEXP e = CAR(cell);
  int argc = Rf_length(call_args(e));
  RAnnot::ExpressionInfo * ei = getProperty(RAnnot::ExpressionInfo, cell);
  Expression out = op_arglist_rec(sb, call_args(e), ei, 0, unprotcnt, laziness_string, rho);
  return out;
}

static Expression op_arglist_rec(SubexpBuffer * const sb, const SEXP args, const RAnnot::ExpressionInfo * const ei, 
				 const int n, int * const unprotcnt, string & laziness_string, const string rho)
{
  if (args == R_NilValue) {
    return Expression::nil_exp;
  }

  Expression tail_exp = op_arglist_rec(sb, CDR(args), ei, n+1, unprotcnt, laziness_string, rho);
  Expression head_exp;
  if (ei->get_eager_lazy(n) == EAGER) {
    head_exp = sb->op_exp(args, rho, Protected, false);  // false: output code
    laziness_string += "E";
  } else {
    head_exp = sb->op_literal(CAR(args), rho);
    if (!head_exp.del_text.empty()) (*unprotcnt)++;
    string prom = sb->appl2("mkPROMISE", to_string(CAR(args)), head_exp.var, rho);
    head_exp = Expression(prom, head_exp.dependence, head_exp.visibility, unp(prom));
    laziness_string += "L";
  }
  string out = sb->appl2("cons", "", head_exp.var, tail_exp.var);
  if (!head_exp.del_text.empty()) (*unprotcnt)++;
  if (!tail_exp.del_text.empty()) (*unprotcnt)++;
  return Expression(out, DEPENDENT, INVISIBLE, unp(out));
}

/// Output the actual argument list as a list of promises to be passed to applyClosure
static Expression op_promise_args(SubexpBuffer * sb, string args1var, SEXP args,
				  int * unprotcnt, string rho) {
  string arglist;
  string cleanup;
  if (args == R_NilValue) {
    arglist = "R_NilValue";
  } else {
#ifdef CALL_BY_VALUE
    Expression arg_value = sb->op_list(args, rho, Protected, false);  // false: output as non-literal
    if (!arg_value.del_text.empty()) (*unprotcnt)++;
    arglist = arg_value.var;
#else  // call by need, the usual R semantics
    arglist = sb->appl2("promiseArgs", "", args1var, rho);
    (*unprotcnt)++;
#endif
  }
  return Expression(arglist, DEPENDENT, INVISIBLE, "");
}
