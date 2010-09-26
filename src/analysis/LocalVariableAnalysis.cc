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

// File: LocalVariableAnalysis.h
//
// Given an SEXP, traverses the expression to discover variable
// information that can be gained locally: use/def, local/free,
// function/argument.
//
// Important note: expressions do not extend across basic block
// boundaries. This means we are not descending into the bodies of
// loops or the true and false bodies of if statements; those are
// considered separate expressions with separate LocalVariableAnalysis
// objects. But we are descending into compound statements like
// fundefs and curly-brace statement lists.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <assert.h>

#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Var.h>
#include <analysis/DefVar.h>
#include <analysis/UseVar.h>

#include "LocalVariableAnalysis.h"

using namespace RAnnot;

// ----- type definitions for readability -----

typedef LocalVariableAnalysis::const_use_iterator const_use_iterator;
typedef LocalVariableAnalysis::const_def_iterator const_def_iterator;
typedef LocalVariableAnalysis::const_call_site_iterator const_call_site_iterator;

LocalVariableAnalysis::LocalVariableAnalysis(const SEXP _stmt)
  : m_stmt(_stmt), m_uses(), m_defs()
{}

/// Traverse the given SEXP and set variable annotations with local
/// syntactic information.
void LocalVariableAnalysis::perform_analysis() {
  build_ud_rhs(m_stmt, Var::Var_MUST, true);  // we're not in an actual arg to any function, so must-use
  // pass true as last argument: initial call to build_ud_rhs, not recursive call
}

const_use_iterator LocalVariableAnalysis::begin_uses() const {
  return m_uses.begin();
}

const_use_iterator LocalVariableAnalysis::end_uses() const {
  return m_uses.end();
}

const_def_iterator LocalVariableAnalysis::begin_defs() const {
  return m_defs.begin();
}

const_def_iterator LocalVariableAnalysis::end_defs() const {
  return m_defs.end();
}

const_call_site_iterator LocalVariableAnalysis::begin_call_sites() const {
  return m_call_sites.begin();

}
const_call_site_iterator LocalVariableAnalysis::end_call_sites() const {
  return m_call_sites.end();
}

/// Traverse the given SEXP (not an lvalue) and set variable
/// annotations with local syntactic information.
/// cell          = a cons cell whose CAR is the expression we're talking about
/// may_must_type = whether var uses should be may-use or must-use
///                 (usually must-use; may-use if we're looking at an actual argument
///                 to a call-by-need function)
/// is_stmt       = true if we're calling build_ud_rhs from outside, false if sub-SEXP
void LocalVariableAnalysis::build_ud_rhs(const SEXP cell, Var::MayMustT may_must_type, bool is_stmt) {
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  if (is_const(e)) {
    // ignore
  } else if (e == R_MissingArg) {
    // ignore
  } else if (is_var(e)) {
    m_uses.push_back(new UseVar(cell, UseVar::UseVar_ARGUMENT, Var::Var_MUST, Locality::Locality_TOP));
  } else if (is_local_assign(e)) {
    build_ud_lhs(assign_lhs_c(e), assign_rhs_c(e), Var::Var_MUST, IN_LOCAL_ASSIGN);
    build_ud_rhs(assign_rhs_c(e), Var::Var_MUST, false);
  } else if (is_free_assign(e)) {
    build_ud_lhs(assign_lhs_c(e), assign_rhs_c(e), Var::Var_MUST, IN_FREE_ASSIGN);
    build_ud_rhs(assign_rhs_c(e), Var::Var_MUST, false);
  } else if (is_fundef(e)) {
    // ignore
  } else if (is_struct_field(e)) {
    build_ud_rhs(CDR(e), Var::Var_MUST, false);
  } else if (is_subscript(e)) {
    m_call_sites.push_back(cell);
    // use of '[' symbol
    m_uses.push_back(new UseVar(e, UseVar::UseVar_FUNCTION, Var::Var_MUST, Locality::Locality_TOP));
    // left side of subscript
    build_ud_rhs(subscript_lhs_c(e), Var::Var_MUST, false);
    // subscript args
    for (SEXP sub_c = subscript_first_sub_c(e); sub_c != R_NilValue; sub_c = CDR(sub_c)) {
      build_ud_rhs(sub_c, Var::Var_MUST, false);
    }
  } else if (is_if(e)) {
    build_ud_rhs(if_cond_c(e), Var::Var_MUST, false);
    if (!is_stmt) {
      build_ud_rhs(if_truebody_c(e), Var::Var_MAY, false);
      if (CAR(if_falsebody_c(e)) != R_NilValue) build_ud_rhs(if_falsebody_c(e), Var::Var_MAY, false);
    }
  } else if (is_for(e)) {
    // defines the induction variable, uses the range
    build_ud_lhs(for_iv_c(e), for_range_c(e), Var::Var_MUST, IN_LOCAL_ASSIGN);
    build_ud_rhs(for_range_c(e), Var::Var_MUST, false);
  } else if (is_loop_header(e)) {
    // TODO
    // currently this case cannot happen
  } else if (is_loop_increment(e)) {
    // TODO
    // currently this case cannot happen
  } else if (is_while(e)) {
    build_ud_rhs(while_cond_c(e), Var::Var_MUST, false);
  } else if (is_repeat(e)) {
    // ignore
  } else if (is_paren_exp(e)) {
    build_ud_rhs(paren_body_c(e), Var::Var_MUST, false);
  } else if (is_curly_list(e)) {
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      build_ud_rhs(stmt, Var::Var_MUST, false);
    }
  } else if (TYPEOF(e) == LANGSXP) {   // regular function call
    m_call_sites.push_back(cell);
    if (is_var(CAR(e))) {
      m_uses.push_back(new UseVar(e, UseVar::UseVar_FUNCTION, Var::Var_MUST, Locality::Locality_TOP));
    } else {
      build_ud_rhs(e, Var::Var_MUST, false);
    }
    // recur on args
    // Because R functions are call-by-need, var uses that appear in
    // args passed to them are MAY-use. BUILTINSXP functions are CBV,
    // so vars passed as args should stay MUST.
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      if (is_var(CAR(e)) && TYPEOF(Rf_findFunUnboundOK(CAR(e), R_GlobalEnv, TRUE)) == BUILTINSXP) {
	build_ud_rhs(stmt, Var::Var_MUST, false);
      } else {
	build_ud_rhs(stmt, Var::Var_MAY, false);
      }
    }
  } else {
    assert(0);
  }  
}

/// Traverse the SEXP contained in the given cons cell, assuming that
/// it's an lvalue. Set variable annotations with local syntactic
/// information.
/// cell          = a cons cell whose CAR is the lvalue we're talking about
/// rhs           = a cons cell whose CAR is the right side of the assignment statement
/// may_must_type = whether we are in a may-def or a must-def
/// lhs_type      = whether we are in a local or free assignment
void LocalVariableAnalysis::build_ud_lhs(const SEXP cell, const SEXP rhs_c,
					 Var::MayMustT may_must_type, LhsType lhs_type)
{
  assert(is_cons(cell));
  assert(is_cons(rhs_c));
  SEXP e = CAR(cell);
  if (is_var(e) || Rf_isString(e)) {
    if (Rf_isString(e)) {
      SETCAR(cell, Rf_install(CHAR(STRING_ELT(e, 0))));
      e = CAR(cell);
    }
    Locality::LocalityType locality;
    if (lhs_type == IN_LOCAL_ASSIGN) {
      locality = Locality::Locality_LOCAL;
    } else {
      locality = Locality::Locality_FREE;
    }
    m_defs.push_back(new DefVar(cell, DefVar::DefVar_ASSIGN, may_must_type, locality, rhs_c));
  } else if (is_struct_field(e)) {
    build_ud_lhs(struct_field_lhs_c(e), rhs_c, Var::Var_MAY, lhs_type);
  } else if (is_subscript(e)) {
    build_ud_lhs(subscript_lhs_c(e), rhs_c, Var::Var_MAY, lhs_type);
    for (SEXP sub_c = subscript_first_sub_c(e); sub_c != R_NilValue; sub_c = CDR(sub_c)) {    
      build_ud_rhs(sub_c, Var::Var_MUST, false);
    }
  } else if (TYPEOF(e) == LANGSXP) {  // regular function call
    // Function application as lvalue. Examples: dim(x) <- foo, attr(x, "dim") <- c(2, 5)
    //
    // TODO: We should really be checking if the function is valid;
    // only some functions applied to arguments make a valid lvalue.
    build_ud_rhs(e, Var::Var_MUST, false);                 // assignment function
    build_ud_lhs(CDR(e), rhs_c, Var::Var_MAY, lhs_type);   // first arg is lvalue
    SEXP arg = CDDR(e);                                    // other args are rvalues if they exist
    while (arg != R_NilValue) {
      build_ud_rhs(arg, Var::Var_MUST, false);
      arg = CDR(arg);
    }
  } else {
    assert(0);
  }
}
