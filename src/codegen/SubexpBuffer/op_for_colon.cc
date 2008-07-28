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
  decls += "double step, begin, end, one_past_end;\n";
  if (!has_i) {
    decls += "int i;\n";
    has_i = TRUE;
  }
  LoopContext this_loop;
  Expression defIV = op_var_def(sym_c, "R_NilValue", rho);
  Expression range_begin = op_exp(call_args(range), rho);
  Expression range_end = op_exp(CDR(call_args(range)), rho);
  string header;
  header += emit_assign("v", emit_call2("allocVector", "REALSXP", "1"), Protected);
  header += emit_assign("begin", emit_call1("REAL", range_begin.var) + "[0]");
  header += emit_assign("end", emit_call1("REAL", range_end.var) + "[0]");
  header += "step = (begin <= end ? 1 : -1);\n";
  header += "one_past_end = end + step;\n";
  header += "for (i = begin; i != one_past_end; i += step) {\n";
  header += "REAL(v)[0] = i;\n";
  header += emit_call3("setVar", make_symbol(CAR(sym_c)), "v", rho) + ";\n";
  append_defs(header);
  Expression ans = op_exp(for_body_c(e), rho, Unprotected, false, resultStatus);
  append_defs("}\n");
  append_defs(this_loop.breakLabel() + ":;\n");
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
