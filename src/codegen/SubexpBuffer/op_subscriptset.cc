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

  string subassign;
  Expression s;
  SEXP lhs = CAR(assign_lhs_c(e));
  SEXP array_c = subscript_lhs_c(lhs);
  Expression a_sym = op_literal(CAR(array_c), rho);
  int unprotcnt = 0;
  Expression a = op_exp(array_c, rho, Protected, true);  // fully evaluated; need to force promise
  Expression r = op_exp(assign_rhs_c(e), rho);
  if (!a.del_text.empty()) unprotcnt++;
  if (!r.del_text.empty()) unprotcnt++;
  switch(Rf_length(subscript_subs(lhs))) {
  case 0:
    s = Expression::bogus_exp;
    subassign = appl2("rcc_subassign_0", a.var, r.var, Unprotected);
    break;
  case 1:
    s = op_exp(subscript_first_sub_c(lhs), rho);
    if (!s.del_text.empty()) unprotcnt++;
    subassign = appl3("rcc_subassign_1", a.var, s.var, r.var, Unprotected);
    break;
    // TODO: write rcc_subassign_2; need deconsed version of MatrixAssign
  default:
    s = op_list(subscript_subs(lhs), rho, false, Protected);
    if (!s.del_text.empty()) unprotcnt++;
    subassign = appl3("rcc_subassign_cons", a.var, s.var, r.var, Unprotected);
    break;
  }
  // the result of the subassign is unprotected because it is
  // immediately protected by the following defineVar
  append_defs(emit_call3("defineVar", a_sym.var, subassign, rho) + ";\n");
  if (unprotcnt > 0) {
    append_defs(emit_call1("UNPROTECT", i_to_s(unprotcnt)) + ";\n");
  }
  r.visibility = INVISIBLE;
  r.del_text.clear();        // we have already unprotected everything necessary
  return r;
}
