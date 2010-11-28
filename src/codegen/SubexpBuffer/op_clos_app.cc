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
#include <analysis/CallByValueInfo.h>
#include <analysis/CallByValueInfoAnnotationMap.h>
#include <analysis/CEscapeInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/OEscapeInfo.h>
#include <analysis/OEscapeInfoAnnotationMap.h>
#include <analysis/ResolvedArgs.h>
#include <analysis/ResolvedArgsAnnotationMap.h>
#include <analysis/ResolvedCallByValueInfo.h>
#include <analysis/ResolvedCallByValueInfoAnnotationMap.h>
#include <analysis/Settings.h>
#include <analysis/Utils.h>
#include <analysis/VFreshDFSolver.h>

#include <support/StringUtils.h>

#include <CodeGen.h>
#include <CodeGenUtils.h>
#include <Metrics.h>
#include <Visibility.h>

using namespace std;
using namespace RAnnot;

#if 0
not used
static Expression op_arglist(SubexpBuffer * sb,
			     const SEXP cell,
			     int * const unprotcnt,
			     string & laziness_string,
			     const string rho);
#endif

static Expression op_arg(SubexpBuffer * const sb,
			 const SEXP cell,
			 const EagerLazyT eager_lazy,
			 const string rho);
			 
static Expression op_arglist_rec(SubexpBuffer * const sb,
				 const SEXP args,
				 const std::vector<EagerLazyT> & lazy_info,
				 const int n,
				 int * const unprotcnt,
				 string & laziness_string,
				 const string rho);
static Expression op_resolved_args(SubexpBuffer * sb,
				   ResolvedArgs * resolved_args,
				   ResolvedCallByValueInfo * cbv,
				   string rho,
				   int * unprotect,
				   string & laziness_string);
static Expression op_resolved_arg(SubexpBuffer * sb,
				  ResolvedArgs * resolved_args,
				  ResolvedCallByValueInfo * cbv,
				  string rho,
				  int * unprotcnt,
				  string & laziness_string,
				  ResolvedArgs::const_reverse_iterator it,
				  int i,
				  Expression tail);
static Expression op_promise_args(SubexpBuffer * sb,
				  string args1var,
				  SEXP args,
				  int * unprotcnt,
				  string rho);
static bool safe_to_stack_alloc_env(SEXP e);

/// Output an application of a closure to actual arguments.
Expression SubexpBuffer::op_clos_app(FuncInfo * fi_if_known,
				     Expression op1,
				     SEXP cell,
				     string rho,
				     Protection resultProtection,
				     EagerLazyT laziness)
{
  SEXP e = CAR(cell);
  SEXP original_args = call_args(e);
  SEXP args;
  int unprotcnt = 0;
  string laziness_string;  // string of E's and L's, one for each arg, saying whether it's eager or lazy
  Expression args1;
  string fallback = "INVALID";
  ResolvedArgs * args_annot;
  std::vector<EagerLazyT> lazy_info;
  bool args_resolved;

  if (Settings::instance()->get_resolve_arguments() &&
      ResolvedArgsAnnotationMap::instance()->is_valid(cell)) {
    args_resolved = true;
    args_annot = getProperty(ResolvedArgs, cell);
    ResolvedCallByValueInfo * cbv = getProperty(ResolvedCallByValueInfo, cell);
    lazy_info = cbv->get_eager_lazy_info();
    args1 = op_resolved_args(this, args_annot, cbv, rho, &unprotcnt, laziness_string);
  } else {
    args_resolved = false;
    lazy_info = getProperty(CallByValueInfo, cell)->get_eager_lazy_info();
    args = original_args;
    if (laziness == EAGER) {
      args1 = op_list(args, rho, false, Protected);   // pass false to output compiled list
      laziness_string = "eager";
    } else {
      args1 = op_arglist_rec(this, args, lazy_info, 0, &unprotcnt, laziness_string, rho);
    }
  }

  string call_str = appl2("lcons", "", op1.var, args1.var);
  unprotcnt++;  // call_str
  string apply_closure_string = "applyClosureOpt ";

  string options;
  if (fi_if_known == 0) {    // library procedure
    options = "AC_DEFAULT";
    if (safe_to_stack_alloc_env(e)) {
      options += " | AC_STACK_CLOSURE";
    }
  } else {
    options = "AC_RCC | AC_ENVIRONMENT | AC_USEMETHOD";
    if (!getProperty(CEscapeInfo, fi_if_known->get_sexp())->may_escape()) {
      options += " | AC_STACK_CLOSURE";
    }
  }
  if (!args_resolved) {
    options += " | AC_MATCH_ARGS";
  }

  /*
  string name;
  if (TYPEOF(CAR(e)) == SYMSXP) {
    name = quote(var_name(CAR(e)));
  } else {
    name = "NULL";
  }
  */
  string callee_sym;
  if (TYPEOF(CAR(e)) == SYMSXP) {
    callee_sym = make_symbol(CAR(e));
  } else {
    callee_sym = make_symbol(Rf_install("*anonymous*"));
  }

  bool may_escape = getProperty(OEscapeInfo, cell)->may_escape();

  if (may_escape) {
    fallback = new_var_unp();
    append_decls("Rboolean " + fallback + ";\n");
    append_defs(emit_assign(fallback, "getFallbackAlloc()"));
    append_defs(emit_call1("setFallbackAlloc","TRUE") + ";\n");
  }
  // Unlike most R internal functions, applyClosure actually uses its
  // 'call' argument, so we can't just make it R_NilValue.
  string out = appl7(apply_closure_string,
		     "op_clos_app: " + to_string(e) + " " + laziness_string,
		     call_str,
		     op1.var,
		     args1.var,
		     rho,
		     "R_NilValue",
		     options,
		     callee_sym,
		     Unprotected);
  if (may_escape) {
    append_defs(emit_call1("setFallbackAlloc", fallback) + ";\n");
  }
  if (!op1.del_text.empty()) unprotcnt++;
  if (!args1.del_text.empty()) unprotcnt++;
  append_defs("UNPROTECT(" + i_to_s(unprotcnt) + ");\n");
  string cleanup;
  if (resultProtection == Protected) {
    append_defs("SAFE_PROTECT(" + out + ");\n");
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

static bool safe_to_stack_alloc_env(SEXP e) {
  // As far as I know, no library procedures can have nested functions
  // that outlive their enclosed scope.
  return true;
}

#if 0
not used

static Expression op_arglist(SubexpBuffer * const sb, const SEXP cell, int * const unprotcnt, string & laziness_string, string rho) {
  SEXP e = CAR(cell);
  ExpressionInfo * ei = getProperty(ExpressionInfo, cell);
  Expression out = op_arglist_rec(sb, call_args(e), 0, unprotcnt, laziness_string, rho);
  return out;
}
#endif

static Expression op_arg(SubexpBuffer * const sb,
			 const SEXP cell,
			 const EagerLazyT eager_lazy,
			 const string rho)
{
  Expression out;
  if (eager_lazy == EAGER && Settings::instance()->get_strictness()) {
    out = sb->op_exp(cell, rho, Protected, false);  // false: output code
    Metrics::instance()->inc_eager_actual_args();
  } else {
    out = sb->op_literal(CAR(cell), rho);
    string prom = sb->appl2("mkPROMISE", to_string(CAR(cell)), out.var, rho);
    out = Expression(prom, out.dependence, out.visibility, unp(prom));
    Metrics::instance()->inc_lazy_actual_args();
  }
  return out;
}

static Expression op_arglist_rec(SubexpBuffer * const sb,
				 const SEXP args,
				 const std::vector<EagerLazyT> & lazy_info,
				 const int n,
				 int * const unprotcnt,
				 string & laziness_string,
				 const string rho)
{
  if (args == R_NilValue) {
    return Expression::nil_exp;
  }

  Expression tail_exp = op_arglist_rec(sb, CDR(args), lazy_info, n+1, unprotcnt, laziness_string, rho);
  EagerLazyT eager_lazy = (lazy_info[n] == EAGER && Settings::instance()->get_strictness()) ? EAGER : LAZY;
  Expression head_exp = op_arg(sb, args, eager_lazy, rho);
  laziness_string = (eager_lazy == EAGER ? "E" : "L") + laziness_string;
  string out;
  if (TAG(args) == R_NilValue) {
    out = sb->appl2("cons", "", head_exp.var, tail_exp.var);
  } else {
    out = sb->appl3("tagged_cons", "", head_exp.var, make_symbol(TAG(args)), tail_exp.var);
  }
  if (!head_exp.del_text.empty()) (*unprotcnt)++;
  if (!tail_exp.del_text.empty()) (*unprotcnt)++;
  return Expression(out, DEPENDENT, INVISIBLE, unp(out));
}

/// Output the actual argument list as a list of promises to be passed to applyClosure
static Expression op_promise_args(SubexpBuffer * sb, string args1var, SEXP args,
				  int * unprotcnt, string rho)
{
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

static Expression op_resolved_args(SubexpBuffer * sb,
				   ResolvedArgs * resolved_args,
				   ResolvedCallByValueInfo * cbv,
				   string rho,
				   int * unprotcnt,
				   string & laziness_string)
{
  int i = 0;
  string out;
  Expression tail = Expression::nil_exp;
  for (ResolvedArgs::const_reverse_iterator it = resolved_args->rbegin();
       it != resolved_args->rend();
       it++)
    {
      tail = op_resolved_arg(sb, resolved_args, cbv, rho, unprotcnt, laziness_string, it, i, tail);
      i++;
    }
  return tail;
}

static Expression op_resolved_arg(SubexpBuffer * sb,
				  ResolvedArgs * resolved_args,
				  ResolvedCallByValueInfo * cbv,
				  string rho,
				  int * unprotcnt,
				  string & laziness_string,
				  ResolvedArgs::const_reverse_iterator it,
				  int i,
				  Expression tail)
{
  string out;
  Expression arg;
  laziness_string = (cbv->get_eager_lazy(i) == EAGER ? "E" : "L") + laziness_string;
  if (it->source == ResolvedArgs::RESOLVED_DEFAULT) {
    arg = sb->op_literal(CAR(it->cell), rho);
    out = sb->appl3("tagged_cons", "", arg.var, make_symbol(TAG(it->formal)), tail.var);
  } else {
    arg = op_arg(sb, it->cell, cbv->get_eager_lazy(i), rho);
    if (TAG(it->cell) == R_NilValue) {
      out = sb->appl2("cons", "", arg.var, tail.var);
    } else {
      out = sb->appl3("tagged_cons", "", arg.var, make_symbol(TAG(it->cell)), tail.var);
    }
  }
  if (!arg.del_text.empty()) (*unprotcnt)++;
  if (!tail.del_text.empty()) (*unprotcnt)++;
  return Expression(out, DEPENDENT, INVISIBLE, unp(out));
}
