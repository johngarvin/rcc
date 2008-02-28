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

// File: Utils.cc
//
// Simple utilities for analyzing SEXPs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <cassert>

#include <analysis/Utils.h>

//--------------------------------------------------------------------
// assignments
//--------------------------------------------------------------------

bool is_assign_prim(const SEXP e) {
  return (TYPEOF(e) == SPECIALSXP && PRIMFUN(e) == (CCODE)do_set);
}

bool is_local_assign_prim(const SEXP e) {
  assert(is_assign_prim(e));
  return (PRIMVAL(e) == 1 || PRIMVAL(e) == 3);
}

bool is_free_assign_prim(const SEXP e) {
  assert(is_assign_prim(e));
  return (PRIMVAL(e) == 2);
}

bool is_local_assign(const SEXP e) {
  return (TYPEOF(e) == LANGSXP
	  && (CAR(e) == Rf_install("<-")
	      || CAR(e) == Rf_install("=")));
}

bool is_free_assign(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("<<-"));
}

bool is_assign(const SEXP e) {
  return (is_local_assign(e) || is_free_assign(e));
}

// assignment where the LHS is a symbol (not an array subscript, etc.)
bool is_simple_assign(const SEXP e) {
  return (is_assign(e) && TYPEOF(CADR(e)) == SYMSXP);
}

SEXP assign_lhs_c(const SEXP e) {
  assert(is_assign(e));
  return CDR(e);
}

SEXP assign_rhs_c(const SEXP e) {
  assert(is_assign(e));
  return CDDR(e);
}

//--------------------------------------------------------------------
// function definitions
//--------------------------------------------------------------------

bool is_fundef(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("function"));
}

SEXP fundef_args_c(const SEXP e) {
  assert(is_fundef(e));
  return CDR(e);
}

SEXP fundef_body_c(const SEXP e) {
  assert(is_fundef(e));
  return CDDR(e);
}

//--------------------------------------------------------------------
// expressions
//--------------------------------------------------------------------

bool is_struct_field(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("$"));
}

SEXP struct_field_lhs_c(const SEXP e) {
  assert(is_struct_field(e));
  return CDR(e);
}

SEXP struct_field_rhs_c(const SEXP e) {
  assert(is_struct_field(e));
  return CDDR(e);
}

// single-bracket subscript expression where left side is a symbol and
// there's only one subscript
bool is_simple_subscript(const SEXP e) {
  return (is_call(e) &&
	  CAR(e) == Rf_install("[") &&
	  is_var(CADR(e)) &&
	  Rf_length(e) == 3);
}

bool is_subscript(const SEXP e) {
  return (TYPEOF(e) == LANGSXP 
	  && (CAR(e) == Rf_install("[")
	      || CAR(e) == Rf_install("[[")));
}

SEXP subscript_lhs_c(const SEXP e) {
  assert(is_subscript(e));
  return CDR(e);
}

SEXP subscript_rhs_c(const SEXP e) {
  assert(is_subscript(e));
  return CDDR(e);
}

bool is_const(const SEXP e) {
  return (   TYPEOF(e) == NILSXP
	  || TYPEOF(e) == REALSXP
	  || TYPEOF(e) == LGLSXP
	  || TYPEOF(e) == STRSXP
	  || TYPEOF(e) == INTSXP
	  || TYPEOF(e) == CPLXSXP);
}

bool is_var(const SEXP e) {
  return (TYPEOF(e) == SYMSXP);
}

std::string var_name(const SEXP e) {
  assert(is_var(e));
  return CHAR(PRINTNAME(e));
}

bool is_library(const SEXP e) {
  assert(is_var(e));
  SEXP fun = Rf_findFunUnboundOK(e, R_GlobalEnv, TRUE);
  return (fun != R_UnboundValue);
}

SEXP library_value(const SEXP e) {
  SEXP value = Rf_findFunUnboundOK(e, R_GlobalEnv, TRUE);
  assert(value != R_UnboundValue);
  return value;
}

bool is_cons(const SEXP e) {
  return (TYPEOF(e) == LISTSXP || TYPEOF(e) == LANGSXP);
}

bool is_string(const SEXP e) {
  return (TYPEOF(e) == STRSXP);
}

bool is_call(const SEXP e) {
  return (TYPEOF(e) == LANGSXP);
}

SEXP call_lhs(const SEXP e) {
  assert(is_call(e));
  return CAR(e);
}

SEXP call_args(const SEXP e) {
  assert(is_call(e));
  return CDR(e);
}

SEXP call_nth_arg(const SEXP e, int n) {
  SEXP args = call_args(e);
  assert(n > 0);
  assert(Rf_length(args) <= n);
  for(int i = 0; i<(n-1); i++) {  // take the CDR (n-1) times
    args = CDR(args);
  }
  return args;
}

//--------------------------------------------------------------------
// control flow statements
//--------------------------------------------------------------------

bool is_if(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("if"));
}

SEXP if_cond_c(const SEXP e) {
  assert(is_if(e));
  return CDR(e);
}

SEXP if_truebody_c(const SEXP e) {
  assert(is_if(e));
  return CDDR(e);
}

SEXP if_falsebody_c(const SEXP e) {
  assert(is_if(e));
  return CDR(CDDR(e));
}

bool is_return(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("return"));
}

bool is_break(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("break"));
}

bool is_stop(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("stop"));
}

bool is_next(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("next"));
}

//--------------------------------------------------------------------
// loops
//--------------------------------------------------------------------

bool is_loop(const SEXP e) {
  return (is_for(e) || is_while(e) || is_repeat(e));
}

bool is_loop_header(const SEXP e) {
  // TODO: is this right?
  return false;
}

bool is_loop_increment(const SEXP e) {
  // TODO: is this right?
  return false;
}

SEXP loop_body_c(const SEXP e) {
  assert(is_loop(e));
  if (is_for(e)) {
    return for_body_c(e);
  } else if (is_while(e)) {
    return while_body_c(e);
  } else if (is_repeat(e)) {
    return repeat_body_c(e);
  } else {
    assert(0);
  }
  return 0;
}

bool is_for(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("for"));
}

/// Returns the CONS containing the induction variable of a for loop.
SEXP for_iv_c(const SEXP e) {
  assert(is_for(e));
  return CDR(e);
}

/// Returns the CONS containing the range of the loop.
SEXP for_range_c(const SEXP e) {
  assert(is_for(e));
  return CDDR(e);
}

/// Returns the CONS containing the body of the given for loop.
SEXP for_body_c(const SEXP e) {
  assert(is_for(e));
  return CDR(CDDR(e));
}

// true if the range of the for loop is a colon expression, like
// 'for (i in 1:n) ...' or 'for (a in (n+1):(m+k/2))'
bool is_for_colon(const SEXP e) {
  assert(is_for(e));
  return (TYPEOF(CAR(for_range_c(e))) == LANGSXP &&
	  CAR(CAR(for_range_c(e))) == Rf_install(":"));
}

bool is_while(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("while"));
}

/// Returns the CONS containing the conditional of a while loop.
SEXP while_cond_c(const SEXP e) {
  assert(is_while(e));
  return CDR(e);
}

/// Returns the CONS containing the body of a while loop.
SEXP while_body_c(const SEXP e) {
  assert(is_while(e));
  return CDDR(e);
}

bool is_repeat(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("repeat"));
}

/// Returns the CONS containing the body of a repeat loop.
SEXP repeat_body_c(const SEXP e) {
  assert(is_repeat(e));
  return CDR(e);
}

//--------------------------------------------------------------------
// Containers (parentheses and curly braces)
// In R, curly braces create a list of expressions.
//--------------------------------------------------------------------

bool is_paren_exp(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("("));
}

SEXP paren_body_c(const SEXP e) {
  assert(is_paren_exp(e));
  return CDR(e);
}

bool is_curly_list(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("{"));
}

SEXP curly_body(const SEXP e) {
  assert(is_curly_list(e));
  return CDR(e);
}

//--------------------------------------------------------------------
// rcc_assert statements
//--------------------------------------------------------------------

bool is_rcc_assert_def(const SEXP e) {
  return (is_simple_assign(e) && is_assert_sym(CAR(assign_lhs_c(e))));
}

bool is_rcc_assertion(const SEXP e) {
  return (is_call(e) && is_assert_sym(call_lhs(e)));
}

bool is_rcc_assert(const SEXP e) {
  return (TYPEOF(e) == LANGSXP 
	  && CAR(e) == Rf_install(".rcc.assert"));
}

bool is_rcc_assert_sym(const SEXP e) {
  return (TYPEOF(e) == LANGSXP 
	  && CAR(e) == Rf_install(".rcc.assert.sym"));
}

bool is_rcc_assert_exp(const SEXP e) {
  return (TYPEOF(e) == LANGSXP 
	  && CAR(e) == Rf_install(".rcc.assert.exp"));
}

bool is_assert_sym(const SEXP e) {
  return (TYPEOF(e) == SYMSXP &&
	  (e == Rf_install(".rcc.assert") ||
	   e == Rf_install(".rcc.assert.sym") ||
	   e == Rf_install(".rcc.assert.exp")));
}

bool is_value_assert(const SEXP e) {
  return (TYPEOF(e) == LANGSXP
	  && (CAR(e) == Rf_install("value")));
}
