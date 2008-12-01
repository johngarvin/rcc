// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: op_for_colon.cc
//
// Output a for loop with a colon expression as the range.
// 
// Example:
// for (i in (n-1):(m+k/2)) ...
//
// A colon expression represents an inclusive range, always with
// stride one, incrementing if the second argument is greater,
// otherwise decrementing.
//
// Examples:
// 1:10      -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
// 1:1       -> 1
// 10:1      -> 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
// 1.2:5.5   -> 1.2, 2.2, 3.2, 4.2, 5.2
// 1.2:6.2   -> 1.2, 2.2, 3.2, 4.2, 5.2, 6.2
// 3.5:-1    -> 3.5, 2.5, 1.5, 0.5, -0.5
// 10.1:10.7 -> 10.1
//
// Analysis notes:

// (1) A colon expression always contains at least one value.
// Therefore, if a for-loop-with-colon is executed, the body is always
// executed at least once.
// (2) A colon expression is always a 1D vector; therefore, the
// iteration variable is always a scalar.

// Author: John Garvin (garvin@cs.rice.edu)


#include <string>

#include <CodeGenUtils.h>
#include <LoopContext.h>

#include <analysis/Utils.h>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <support/StringUtils.h>

using namespace std;

Expression SubexpBuffer::op_for_colon(SEXP e, string rho,
				      ResultStatus resultStatus)
{
  SEXP sym_c = for_iv_c(e);
  SEXP range = CAR(for_range_c(e));
  decls += "SEXP v;\n";
  decls += "double step, begin, end;\n";
  decls += "Rboolean count_up;\n";
  if (!has_di) {
    decls += "double di;\n";
    has_di = TRUE;
  }
  LoopContext this_loop;
  Expression defIV = op_var_def(sym_c, "R_NilValue", rho);
  Expression range_begin = op_exp(call_args(range), rho);
  Expression range_end = op_exp(CDR(call_args(range)), rho);
  string header;
  header += emit_assign("v", emit_call2("allocVector", "REALSXP", "1"), Protected);
  string rbv = range_begin.var;
  string rev = range_end.var;
  header += emit_assign("begin",
			emit_call1("TYPEOF", rbv) + " == " + "INTSXP" + " ? " +
			emit_call1("INTEGER", rbv) + "[0]" + " : " +
			emit_call1("REAL", rbv) + "[0]");
  header += emit_assign("end",
			emit_call1("TYPEOF", rev) + " == " + "INTSXP" + " ? " +
			emit_call1("INTEGER", rev) + "[0]" + " : " +
			emit_call1("REAL", rev) + "[0]");
  header += emit_assign("count_up", "(begin <= end)");
  header += "step = (count_up ? 1.0 : -1.0);\n";
  header += "for (di = begin; (count_up ? (di < end + FLT_EPSILON) : (di > end - FLT_EPSILON)); di += step) {\n";
  append_defs(header);
  SubexpBuffer for_body;
  for_body.append_defs("REAL(v)[0] = di;\n");
  for_body.append_defs(emit_call3("setVar", make_symbol(CAR(sym_c)), "v", rho) + ";\n");
  Expression ans = for_body.op_exp(for_body_c(e), rho, Unprotected, false, resultStatus);
  append_decls(indent(for_body.output_decls()));
  append_defs(indent(for_body.output_defs()));
  append_defs("}\n");
  append_defs(this_loop.breakLabel() + ":;\n");
  del(range_begin);
  del(range_end);
  append_defs(emit_call1("UNPROTECT_PTR", "v") + ";\n");
  return Expression(ans.var, DEPENDENT, INVISIBLE, "");
}

#if 0

[ op_var_def(sym_c, "R_NilValue") -> sym ]
[ op_exp(CAR(call_args(range))) -> begin ]
[ op_exp(CADR(call_args(range))) -> end  ]
v = allocVector(TYPE, 1);
TYPE i;
  for (i=begin; i<=end; i++) {
    TYPE(v)[0] = i;
    setVar(sym, v, rho);
    [body]
  }
}

#endif
