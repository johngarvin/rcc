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

// File: op_subscriptset.cc
//
// Output an assignment where the left side is a subscript expression.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <string>

#include <CodeGenUtils.h>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <include/R/R_RInternals.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/StringUtils.h>

#include <Visibility.h>

using namespace std;

Expression SubexpBuffer::op_subscriptset(SEXP e, string rho, 
					 Protection resultProtection)
{
  // The interpreter handles subscript assignments A[B] <- C in the
  // following way:
  //  - stores the current value of the array A in a temporary
  // variable
  //  - creates and executes a call to the subscript operator "[<-",
  // which takes the array, subscript, and right side as arguments and
  // returns the updated array. (R uses a trick with reference
  // counting in the NAMED field to make sure the array is modified in
  // place unless a copy is necessary.)
  //  - assigns the result of the "[<-" call to the original array's name.
  //
  // In other words, the interpreter transforms this:
  //
  // A[B] <- C
  //
  // into this:
  //
  // *tmp* <- A
  // A <- "[<-"(*tmp*, B, value=C)
  //
  // So in RCC we generate code similar to this: evaluate A, B, and C,
  // call our version of the "[<-" operator (rcc_subassign), and
  // assign the result to A.

  SEXP lhs = CAR(assign_lhs_c(e));
  SEXP array_c = subscript_lhs_c(lhs);
  SEXP sub_c = subscript_rhs_c(lhs);
  SEXP rhs_c = assign_rhs_c(e);
  Expression a_sym = op_literal(CAR(array_c), rho);
  Expression a = op_exp(array_c, rho, Protected, true);  // fully evaluated; need to force promise
  Expression s = op_exp(sub_c, rho);
  Expression r = op_exp(rhs_c, rho, resultProtection);
  string subassign = appl3("rcc_subassign", a.var, s.var, r.var, Unprotected);
  // unprotected because immediately followed by the defineVar
  append_defs(emit_call3("defineVar", a_sym.var, subassign, rho) + ";\n");
  del(a_sym);
  del(a);
  del(s);
  r.visibility = INVISIBLE;
  return r;
}
