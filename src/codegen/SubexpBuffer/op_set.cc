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

// File: op_set.cc
//
// Output an assignment statement.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <cassert>
#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_Defn.h>
#include <support/StringUtils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

#include <CodeGen.h>
#include <CodeGenUtils.h>
#include <GetName.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

/// Output an assignment statement
Expression SubexpBuffer::op_set(SEXP e, SEXP op, string rho, 
				Protection resultProtection)
{
  string out;
  assert(is_assign_prim(op));
  SEXP lhs = CAR(assign_lhs_c(e));
  if (is_string(lhs)) {
    SETCAR(CDR(e), Rf_install(CHAR(STRING_ELT(lhs, 0))));
    lhs = CAR(assign_lhs_c(e));
  }
  if (is_var(lhs)) {
    string name = make_symbol(lhs);
    SEXP rhs_c = assign_rhs_c(e);
    SEXP rhs = CAR(assign_rhs_c(e));
    Expression body;
    if (is_fundef(rhs)) {
      body = op_fundef(rhs, rho, resultProtection);
    } else {
      // Right side of an assignment is always evaluated.
      // Pass 'true' to tell op_exp to evaluate the result
      body = op_exp(rhs_c, rho, Protected, true);
    }

    // whether the assignment is local or free
    // determines which environment we should use
    string target_env;
    if (is_local_assign_prim(op) || rho == "R_GlobalEnv") {
      target_env = rho;
    } else {
      target_env = emit_call1("ENCLOS", rho);
    }

    Expression out = op_var_def(assign_lhs_c(e), body.var, target_env);
    if (!body.del_text.empty())
      append_defs("UNPROTECT(1);\n");
    return out;
  } else if (is_simple_subscript(lhs)) {
    return op_subscriptset(e, rho, resultProtection);
  } else if (Rf_isLanguage(lhs)) {
#ifdef USE_OUTPUT_CODEGEN
    Expression func = output_to_expression(CodeGen::op_primsxp(op, rho));
    Expression args = output_to_expression(CodeGen::op_list(CDR(e), rho, true, Protected));
#else
    Expression func = ParseInfo::global_constants->op_primsxp(op, rho);
    Expression args = op_list(CDR(e), rho, true, Protected);
#endif
    out = appl4("do_set",
		"R_NilValue",
		func.var,
		args.var,
		rho, resultProtection);
    string cleanup;
    if (resultProtection == Protected) cleanup = unp(out);
    del(func);
    del(args);
    return Expression(out, DEPENDENT, INVISIBLE, cleanup);
  } else {
    ParseInfo::flag_problem();
    return Expression("<<assignment with unrecognized LHS>>",
		      DEPENDENT, INVISIBLE, "");
  }
}
