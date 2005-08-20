#include "Utils.h"

//--------------------------------------------------------------------
// assignments
//--------------------------------------------------------------------

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

bool is_subscript(const SEXP e) {
  assert(TYPEOF(e) == LANGSXP);
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
}

bool is_for(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("for"));
}

//! Returns the CONS containing the induction variable of a for loop.
SEXP for_iv_c(const SEXP e) {
  assert(is_for(e));
  return CDR(e);
}

//! Returns the CONS containing the range of the loop.
SEXP for_range_c(const SEXP e) {
  assert(is_for(e));
  return CDDR(e);
}

//! Returns the CONS containing the body of the given for loop.
SEXP for_body_c(const SEXP e) {
  assert(is_for(e));
  return CDR(CDDR(e));
}

bool is_while(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("while"));
}

//! Returns the CONS containing the conditional of a while loop.
SEXP while_cond_c(const SEXP e) {
  assert(is_while(e));
  return CDR(e);
}

//! Returns the CONS containing the body of a while loop.
SEXP while_body_c(const SEXP e) {
  assert(is_while(e));
  return CDDR(e);
}

bool is_repeat(const SEXP e) {
  return (TYPEOF(e) == LANGSXP && CAR(e) == Rf_install("repeat"));
}

//! Returns the CONS containing the body of a repeat loop.
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

