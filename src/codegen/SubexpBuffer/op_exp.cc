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

// File: op_exp.cc
//
// Output an SEXP.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <cassert>
#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>
#include <codegen/SubexpBuffer/SplitSubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/StringUtils.h>
#include <support/RccError.h>
#include <CodeGen.h>
#include <ParseInfo.h>
#include <Visibility.h>

using namespace std;

// op_exp
//
// outputs a representation of the CAR of the given SEXP. Note that
// op_exp takes as the argument the CONS cell containing the
// expression instead of the expression itself; this allows op_var to
// use annotations. (Otherwise, we wouldn't be able to distinguish
// different mentions of the same variable.)

Expression SubexpBuffer::op_exp(SEXP cell, string rho, Protection
				resultProtection, bool fullyEvaluatedResult, 
				ResultStatus resultStatus)
{
  assert(is_cons(cell));
  SEXP e = CAR(cell);

  Expression out, formals, body, env;
  switch(TYPEOF(e)) {
  case NILSXP:
    return Expression::nil_exp;
    break;
  case STRSXP:
    return op_string(e);
    break;
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
#ifdef USE_OUTPUT_CODEGEN
      return output_to_expression(CodeGen::op_vector(e));
#else
      return op_vector(e);
#endif
    break;
  case VECSXP:
    ParseInfo::flag_problem();
    return Expression("<<unimplemented vector>>", DEPENDENT, INVISIBLE, "");
    break;
  case SYMSXP:
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_var_use(cell, rho, fullyEvaluatedResult));
#else
    return op_var_use(cell, rho, resultProtection, fullyEvaluatedResult);
#endif
    break;
  case LISTSXP:
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_list(CScope(prefix + "_" + i_to_s(n)), e, rho, false, false));
#else
    return op_list(e, rho, false, resultProtection, false);
#endif
    break;
  case CLOSXP:
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_closure(e, rho));
#else
    return op_closure(e, rho, resultProtection);
#endif
    break;
  case ENVSXP:
    ParseInfo::flag_problem();
    return Expression("<<unexpected environment>>", DEPENDENT, INVISIBLE, "");
    break;
  case PROMSXP:
    ParseInfo::flag_problem();
    return Expression("<<unexpected promise>>", DEPENDENT, INVISIBLE, "");
    break;
  case LANGSXP:
    out = op_lang(e, rho, resultProtection, resultStatus);
    return out;
    break;
  case CHARSXP:
    return Expression(appl1("mkChar",
			    quote(string(CHAR(e))), Unprotected),
		      CONST, VISIBLE, "");
    break;
  case SPECIALSXP:
  case BUILTINSXP:
#ifdef USE_OUTPUT_CODEGEN
    return output_to_expression(CodeGen::op_primsxp(e, rho));
#else 
    return ParseInfo::global_constants->op_primsxp(e, rho);
#endif
    break;
  case EXPRSXP:
  case EXTPTRSXP:
  case WEAKREFSXP:
    ParseInfo::flag_problem();
    return Expression("<<unimplemented type " + i_to_s(TYPEOF(e)) + ">>",
		      DEPENDENT, INVISIBLE, "");
    break;
  default:
    rcc_error("Internal error: op_exp encountered bad type");
    return Expression::bogus_exp; // never reached
    break;
  }
}
