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
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Var.h>
#include <analysis/DefVar.h>
#include <analysis/UseVar.h>

#include "LocalVariableAnalysis.h"

using namespace RAnnot;

// ----- type definitions for readability -----

typedef LocalVariableAnalysis::const_var_iterator const_var_iterator;
typedef LocalVariableAnalysis::const_call_site_iterator const_call_site_iterator;

LocalVariableAnalysis::LocalVariableAnalysis(const SEXP _stmt)
  : m_stmt(_stmt), m_vars()
{}

/// Traverse the given SEXP and set variable annotations with local
/// syntactic information.
void LocalVariableAnalysis::perform_analysis() {
  build_ud_rhs(m_stmt, Var::Var_MUST);  // we're not in an actual arg to any function, so must-use
}

const_var_iterator LocalVariableAnalysis::begin_vars() const { return m_vars.begin(); }
const_var_iterator LocalVariableAnalysis::end_vars() const { return m_vars.end(); }
const_call_site_iterator LocalVariableAnalysis::begin_call_sites() const { return m_call_sites.begin(); }
const_call_site_iterator LocalVariableAnalysis::end_call_sites() const { return m_call_sites.end(); }

/// Traverse the given SEXP (not an lvalue) and set variable
/// annotations with local syntactic information.
/// cell          = a cons cell whose CAR is the expression we're talking about
/// may_must_type = whether var uses should be may-use or must-use
///                 (usually must-use; may-use if we're looking at an actual argument
///                 to a call-by-need function)
void LocalVariableAnalysis::build_ud_rhs(const SEXP cell, Var::MayMustT may_must_type) {
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  if (is_const(e)) {
    // ignore
  } else if (is_var(e)) {
    UseVar * var_annot = new UseVar();
    var_annot->setMention_c(cell);
    var_annot->setPositionType(UseVar::UseVar_ARGUMENT);
    var_annot->setMayMustType(Var::Var_MUST);
    var_annot->setScopeType(Locality_TOP);
    m_vars.push_back(var_annot);
  } else if (is_local_assign(e)) {
    build_ud_lhs(assign_lhs_c(e), assign_rhs_c(e), Var::Var_MUST, IN_LOCAL_ASSIGN);
    build_ud_rhs(assign_rhs_c(e), Var::Var_MUST);
  } else if (is_free_assign(e)) {
    build_ud_lhs(assign_lhs_c(e), assign_rhs_c(e), Var::Var_MUST, IN_FREE_ASSIGN);
    build_ud_rhs(assign_rhs_c(e), Var::Var_MUST);
  } else if (is_fundef(e)) {
    // ignore
  } else if (is_struct_field(e)) {
    build_ud_rhs(CDR(e), Var::Var_MUST);
  } else if (is_subscript(e)) {
    build_ud_rhs(subscript_lhs_c(e), Var::Var_MUST);
    build_ud_rhs(subscript_rhs_c(e), Var::Var_MUST);
  } else if (is_if(e)) {
    build_ud_rhs(if_cond_c(e), Var::Var_MUST);
  } else if (is_for(e)) {
    // defines the induction variable, uses the range
    build_ud_lhs(for_iv_c(e), for_range_c(e), Var::Var_MUST, IN_LOCAL_ASSIGN);
    build_ud_rhs(for_range_c(e), Var::Var_MUST);
  } else if (is_loop_header(e)) {
    // XXXXX
  } else if (is_loop_increment(e)) {
    // XXXXX
  } else if (is_while(e)) {
    build_ud_rhs(while_cond_c(e), Var::Var_MUST);
  } else if (is_repeat(e)) {
    // ignore

    //  } else if (is_paren_exp(e)) {
    //    build_ud_rhs(paren_body_c(e));

  } else if (is_curly_list(e)) {
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      build_ud_rhs(stmt, Var::Var_MUST);
    }
  } else if (TYPEOF(e) == LANGSXP) {   // regular function call
    m_call_sites.push_back(e);
    if (is_var(CAR(e))) {
      UseVar * var_annot = new UseVar();
      var_annot->setMention_c(e);
      var_annot->setPositionType(UseVar::UseVar_FUNCTION);
      var_annot->setMayMustType(Var::Var_MUST);
      var_annot->setScopeType(Locality_TOP);
      m_vars.push_back(var_annot);
    } else {
      // TODO: what about the lhs?
      build_ud_rhs(e, Var::Var_MUST);
    }
    // recur on args
    // Because R functions are call-by-need, var uses that appear in
    // args passed to them are MAY-use. BUILTINSXP functions are CBV,
    // so vars passed as args should stay MUST.
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      if (is_var(CAR(e)) && TYPEOF(Rf_findFunUnboundOK(CAR(e), R_GlobalEnv, TRUE)) == BUILTINSXP) {
	build_ud_rhs(stmt, Var::Var_MUST);
      } else {
	build_ud_rhs(stmt, Var::Var_MAY);
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
  if (is_var(e)) {
    DefVar * var_annot = new DefVar();
    var_annot->setMention_c(cell);
    var_annot->setMayMustType(may_must_type);
    var_annot->setSourceType(DefVar::DefVar_ASSIGN);
    var_annot->setRhs_c(rhs_c);
    // Note this subtlety in the R language. For a must-def, syntactic
    // information (i.e., whether the arrow is single or double) tells
    // us whether we are dealing with a local or free
    // variable. Example: x is local in 'x <- 10' and free in 'x <<-
    // 10'. For a may-def, however, the kind of arrow doesn't matter;
    // locality has to be resolved with data flow analysis. Example:
    // in both 'x[3] <- 10' and 'x[3] <<- 10', we have to look at data
    // flow to figure out the scope of x.
    if (may_must_type == Var::Var_MUST) {
      if (lhs_type == IN_LOCAL_ASSIGN) {
	var_annot->setScopeType(Locality_LOCAL);
      } else {
	var_annot->setScopeType(Locality_FREE);
      }
    } else {                                             // may-def
      var_annot->setScopeType(Locality_TOP);
    }
    m_vars.push_back(var_annot);
  } else if (is_struct_field(e)) {
    build_ud_lhs(struct_field_lhs_c(e), rhs_c, Var::Var_MAY, lhs_type);
  } else if (is_subscript(e)) {
    build_ud_lhs(subscript_lhs_c(e), rhs_c, Var::Var_MAY, lhs_type);
    build_ud_rhs(subscript_rhs_c(e), Var::Var_MUST);
  } else if (TYPEOF(e) == LANGSXP) {  // regular function call
    // Function application as lvalue. For example: dim(x) <- foo
    //
    // FIXME: We should really be checking if the function is valid;
    // only some functions applied to arguments make a valid lvalue.
    assert(CDDR(e) == R_NilValue); // more than one argument is an error, right?
    build_ud_rhs(e, Var::Var_MUST);
    build_ud_lhs(CDR(e), rhs_c, Var::Var_MAY, lhs_type);
  } else {
    assert(0);
  }
}
