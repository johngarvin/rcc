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

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/CallGraphAnnotation.h>
#include <analysis/CallGraphNode.h>
#include <analysis/HandleInterface.h>
#include <analysis/LibraryCallGraphNode.h>
#include <analysis/FundefCallGraphNode.h>
#include <analysis/UnknownValueCallGraphNode.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>
#include <support/StringUtils.h>
#include <support/RccError.h>
#include <Visibility.h>
#include <ParseInfo.h>
#include <CodeGen.h>

using namespace std;
using namespace RAnnot;
using namespace HandleInterface;

static Expression op_internal_call(SubexpBuffer * sb, const LibraryCallGraphNode * lib, SEXP e,
				   string rho, Protection resultProtection, ResultStatus resultStatus);

Expression SubexpBuffer::op_lang(SEXP e, string rho, 
	   Protection resultProtection,
	   ResultStatus resultStatus)
{
  if (TYPEOF(call_lhs(e)) == SYMSXP) {
    // check for SPECIALSXP type
    // the value is conveniently stored in the symbol, so we can just grab it
    // redefinition of specials is forbidden, so no need to check call graph
    if (is_library(call_lhs(e)) && TYPEOF(library_value(call_lhs(e))) == SPECIALSXP) {
      return op_special(e, library_value(call_lhs(e)), rho, resultProtection, resultStatus);
    }
    // see if call graph supplies a single definition
    CallGraphAnnotation * ann = getProperty(CallGraphAnnotation, e);
    const CallGraphNode * node = ann->get_singleton_if_exists();
    if (node) {
      // node is Fundef, Library, or UnknownValue node
      if (const FundefCallGraphNode * cs = dynamic_cast<const FundefCallGraphNode *>(node)) {
	FuncInfo * fi = getProperty(FuncInfo, cs->get_sexp());
	Expression closure_exp = Expression(fi->get_closure(), false, INVISIBLE, "");
	return op_clos_app(closure_exp, call_args(e), rho, resultProtection);
      } else if (const LibraryCallGraphNode * lib = dynamic_cast<const LibraryCallGraphNode *>(node)) {
	return op_internal_call(this, lib, e, rho, resultProtection, resultStatus);
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
  } else {  // left side is not a symbol
    // generate closure and application
    Expression op1;
    op1 = op_exp(e, rho, Unprotected);  // evaluate LHS
    return op_clos_app(op1, call_args(e), rho, resultProtection);
    // eval.c: 395
  }
}

/// Output a call to a library or builtin bound in the R environment.
static Expression op_internal_call(SubexpBuffer * sb, const LibraryCallGraphNode * lib, SEXP e,
				   string rho, Protection resultProtection, ResultStatus resultStatus)
{
  Expression ret_val;
  const SEXP op = lib->get_value();
  if (TYPEOF(op) == CLOSXP) {
    Expression func = sb->op_fun_use(e, rho, resultProtection, false);
    // above: false as last argument for unevaluated result. Is this correct?
    return sb->op_clos_app(func, CDR(e), rho, resultProtection);
  } else if (TYPEOF(op) == BUILTINSXP) {
    return sb->op_builtin(e, op, rho, resultProtection);
  } else {
    rcc_error("Internal error: LANGSXP encountered non-function op");
    return Expression::bogus_exp;
  }
}
