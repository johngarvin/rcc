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

// File: op_return.cc
//
// Output a 'return' expression.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <CheckProtect.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>
#include <analysis/AnalysisResults.h>
#include <analysis/FuncInfo.h>

#include <ParseInfo.h>
#include <Dependence.h>
#include <Visibility.h>
#include <CodeGen.h>

using namespace std;
using namespace RAnnot;

Expression SubexpBuffer::op_return(SEXP e, string rho) {
  FuncInfo *fi = 
    lexicalContext.IsEmpty() ? NULL : lexicalContext.Top();


  Expression value;
  string v;
  switch(Rf_length(e)) {
  case 0:
    v = "R_NilValue";
    value = Expression::nil_exp;
    break;
  case 1:
    // pass true as fourth argument to yield fully evaluated result
    value = op_exp(e, rho, Unprotected, true);
#if 0
    if (value.dependence == DEPENDENT) {
      append_defs("PROTECT(" + value.var + ");\n");
      v = appl2("eval", value.var, rho, Unprotected);
      append_defs("UNPROTECT_PTR(" + value.var + ");\n");
    } else {
      v = value.var;
    }
#else
    v = value.var;
#endif
    del(value);
    break;
  default:
    SEXP exp = e;
    // set names for multiple return
    while(exp != R_NilValue) {
      SEXP tag = TAG(exp);
      if (tag == R_NilValue && TYPEOF(CAR(exp)) == SYMSXP) {
	SET_TAG(exp, CAR(exp));
      }
      exp = CDR(exp);
    }
#ifdef USE_OUTPUT_CODEGEN
    value = output_to_expression(CodeGen::op_list(e, rho, false, true));
#else
    value = op_list(e, rho, false, Protected, true);
#endif
    v = appl1("PairToVectorList", value.var, Unprotected);
    del(value);
    break;
  }

  //---------------------------
  // tear down context, if any
  //---------------------------
  if (fi && fi->requires_context())  
    append_defs("endcontext(&context);\n"); 

  //---------------------------
  // unprotect newenv
  //---------------------------
  append_defs("UNPROTECT(1); /* newenv */\n");
#ifdef CHECK_PROTECT
  append_defs("assert(topval == R_PPStackTop);\n");
#endif

  //---------------------------
  // set visibility
  //---------------------------
  if (value.visibility == VISIBLE) append_defs("R_Visible = 1;");
  if (value.visibility == INVISIBLE) append_defs("R_Visible = 0;");
  // if CHECK_VISIBILITY, then don't change anything

  //---------------------------
  // return v
  //---------------------------
  append_defs("return " + v + ";\n");

  return Expression("R_NilValue", DEPENDENT, INVISIBLE, "");
}
