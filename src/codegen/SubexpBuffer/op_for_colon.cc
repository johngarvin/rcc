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
// Therefore, if a for loop whose range is a colon expression is
// executed, the body is always executed at least once.
// (2) A colon expression is always a 1D vector; its iteration variable

// Author: John Garvin (garvin@cs.rice.edu)


#include <string>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

using namespace std;

Expression SubexpBuffer::op_for_colon(SEXP e, string rho,
				      ResultStatus resultStatus)
{
  return op_for(e, rho, resultStatus);
}

#if 0

[ op_var_def(sym_c, "R_NilValue") -> sym ]
[ op_exp(CAR(call_args(range))) -> begin ]
[ op_exp(CADR(call_args(range))) -> end  ]
v = allocVector(TYPE, 1);
TYPE i;
if (begin <= end) {
  for (i=begin; i<=end; i++) {
    TYPE(v)[0] = i;
    setVar(sym, v, rho);
  }
} else {
  for (i=begin; i>=end; i--) {
    TYPE(v)[0] = i;
    setVar(sym, v, rho);
  }
}

#endif
