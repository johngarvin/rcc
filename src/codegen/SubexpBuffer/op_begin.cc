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

// File: op_begin.cc
//
// Output a sequence of statements.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <support/StringUtils.h>
#include <CodeGenUtils.h>

using namespace std;

//! Output a sequence of statements
Expression SubexpBuffer::op_begin(SEXP exp, string rho,
				  Protection resultProtection,
				  ResultStatus resultStatus)
{
  Expression e;
  string var;
  string cleanup;
  if (exp == R_NilValue) {
    return Expression::nil_exp;
  }
  while (exp != R_NilValue) {
    SEXP next = CDR(exp);
    SubexpBuffer temp = new_sb("tmp_be");
    //    temp.encl_fn = this;
    ResultStatus rs = (next == R_NilValue) ? resultStatus : NoResultNeeded;

    // To be safe, if the expression is a symbol (which might be bound
    // to a promise), evaluate at the end to force the promise.
    // FIXME: This causes unnecessary calls to eval when the
    // expression is not a promise, which we should eventually
    // eliminate.
    bool possible_promise = (TYPEOF(CAR(exp)) == SYMSXP);

    e = temp.op_exp(exp, rho, Unprotected, true, rs);

    string code = temp.output();

    if (next == R_NilValue) {
      if (resultStatus == ResultNeeded) {
	var = new_sexp();
	string assign = emit_assign(var, e.var);
	if (resultProtection == Protected) {
	  assign += protect_str(var) + ";\n";
	  cleanup = unp(var);
	}
	code += assign;
      }
    }
    append_defs(emit_in_braces(code, resultProtection == Unprotected));
    exp = next;
  }
  return Expression(var, e.dependence, e.visibility, cleanup);
}
  
