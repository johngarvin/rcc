// -*- Mode: C++ -*-
//
// Copyright (c) 2010 Rice University
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

// File: SexpTraversal.cc
//
// Traverses an SEXP; fills in both ExpressionInfoAnnotationMap and
// BasicVarAnnotationMap.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/CFG/CFGInterface.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/BasicFuncInfo.h>
#include <analysis/BasicFuncInfoAnnotationMap.h>
#include <analysis/HandleInterface.h>
#include <analysis/SimpleIterators.h>
#include <analysis/Utils.h>

#include "SexpTraversal.h"

static bool debug;

using namespace HandleInterface;

namespace RAnnot {

SexpTraversal * SexpTraversal::instance() {
  if (s_instance == 0) {
    s_instance = new SexpTraversal();
  }
  return s_instance;
}

SexpTraversal::SexpTraversal() {
  BasicFuncInfo * bfi;
  OA::OA_ptr<OA::CFG::NodeInterface> node;
  OA::StmtHandle stmt;

  RCC_DEBUG("RCC_SexpTraversalAnnotationMap", debug);

  FOR_EACH_BASIC_PROC(bfi) {
    // default arguments
    for (SEXP arg = bfi->get_args(); arg != R_NilValue; arg = CDR(arg)) {
      if (CAR(arg) != R_MissingArg) {
	make_expression_info(arg, Locality::Locality_NONE);
	// TODO: should this be TOP and enable local/free analysis for default args?
      }
    }

    PROC_FOR_EACH_NODE(bfi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	// statements
	ExpressionInfo * annot = make_expression_info(make_sexp(stmt), Locality::Locality_TOP);
	if (is_for(CAR(make_sexp(stmt)))) {
	  make_expression_info(for_range_c(CAR(make_sexp(stmt))), Locality::Locality_TOP);
	}
	for(ExpressionInfo::const_call_site_iterator csi = annot->begin_call_sites(); csi != annot->end_call_sites(); ++csi) {
	  // call sites
	  make_expression_info(*csi, Locality::Locality_TOP);
	  // LHS
	  make_expression_info(CAR(*csi), Locality::Locality_TOP);
	  for(R_CallArgsIterator arg_it(CAR(*csi)); arg_it.isValid(); ++arg_it) {
	    // actual arguments
	    make_expression_info(arg_it.current(), Locality::Locality_TOP);
	  }
	  // default args of possible library procedures
	  // if it turns out to be a user proc shadowing a library proc, no harm done
	  SEXP lhs = call_lhs(CAR(*csi));
	  if (is_var(lhs) && is_library(lhs) && is_closure(library_value(lhs))) {
	    for (SEXP formal = closure_args(library_value(lhs)); formal != R_NilValue; formal = CDR(formal)) {
	      make_expression_info(formal, Locality::Locality_NONE);
	    }
	  }
	}
      }
    }
  }
}

ExpressionInfo * SexpTraversal::make_expression_info(const SEXP & k, Locality::LocalityType lt) {
  if (debug) {
    std::cout << "SexpTraversal analyzing the following expression:" << std::endl;
    Rf_PrintValue(CAR(k));
  }

  ExpressionInfo * annot = new ExpressionInfo(k);
  build_ud_rhs(annot, k, BasicVar::Var_MUST, Locality::Locality_TOP, true);
  putProperty(ExpressionInfo, k, annot);
  return annot;
}

/// Traverse the given SEXP (not an lvalue) and set variable
/// annotations with local syntactic information.
/// cell          = a cons cell whose CAR is the expression we're talking about
/// may_must_type = whether var uses should be may-use or must-use
///                 (usually must-use; may-use if we're looking at an actual argument
///                 to a call-by-need function)
/// lt            = whether to initialize variables as TOP or NONE
/// is_stmt       = true if we're calling build_ud_rhs from outside, false if sub-SEXP
void SexpTraversal::build_ud_rhs(ExpressionInfo * ei,
				 const SEXP cell,
				 BasicVar::MayMustT may_must_type,
				 Locality::LocalityType lt,
				 bool is_stmt)
{
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  if (is_const(e)) {
    // ignore
  } else if (e == R_MissingArg) {
    // ignore
  } else if (is_var(e)) {
    make_use_var(ei, cell, UseVar::UseVar_ARGUMENT, BasicVar::Var_MUST, lt);
  } else if (is_local_assign(e)) {
    build_ud_lhs(ei, assign_lhs_c(e), assign_rhs_c(e), BasicVar::Var_MUST, lt, IN_LOCAL_ASSIGN);
    build_ud_rhs(ei, assign_rhs_c(e), BasicVar::Var_MUST, lt, false);
  } else if (is_free_assign(e)) {
    build_ud_lhs(ei, assign_lhs_c(e), assign_rhs_c(e), BasicVar::Var_MUST, lt, IN_FREE_ASSIGN);
    build_ud_rhs(ei, assign_rhs_c(e), BasicVar::Var_MUST, lt, false);
  } else if (is_fundef(e)) {
    // ignore
  } else if (is_struct_field(e)) {
    build_ud_rhs(ei, CDR(e), BasicVar::Var_MUST, lt, false);
  } else if (is_subscript(e)) {
    ei->insert_call_site(cell);
    // use of '[' symbol
    make_use_var(ei, e, UseVar::UseVar_FUNCTION, BasicVar::Var_MUST, lt);
    // left side of subscript
    build_ud_rhs(ei, subscript_lhs_c(e), BasicVar::Var_MUST, lt, false);
    // subscript args
    for (SEXP sub_c = subscript_first_sub_c(e); sub_c != R_NilValue; sub_c = CDR(sub_c)) {
      build_ud_rhs(ei, sub_c, BasicVar::Var_MUST, lt, false);
    }
  } else if (is_if(e)) {
    build_ud_rhs(ei, if_cond_c(e), BasicVar::Var_MUST, lt, false);
    if (!is_stmt) {
      build_ud_rhs(ei, if_truebody_c(e), BasicVar::Var_MAY, lt, false);
      if (CAR(if_falsebody_c(e)) != R_NilValue) {
	build_ud_rhs(ei, if_falsebody_c(e), BasicVar::Var_MAY, lt, false);
      }
    }
  } else if (is_for(e)) {
    // defines the induction variable, uses the range
    build_ud_lhs(ei, for_iv_c(e), for_range_c(e), BasicVar::Var_MUST, lt, IN_LOCAL_ASSIGN);
    build_ud_rhs(ei, for_range_c(e), BasicVar::Var_MUST, lt, false);
  } else if (is_loop_header(e)) {
    assert(0);
    // currently this case cannot happen
  } else if (is_loop_increment(e)) {
    assert(0);
    // currently this case cannot happen
  } else if (is_while(e)) {
    build_ud_rhs(ei, while_cond_c(e), BasicVar::Var_MUST, lt, false);
  } else if (is_repeat(e)) {
    // ignore
  } else if (is_paren_exp(e)) {
    build_ud_rhs(ei, paren_body_c(e), BasicVar::Var_MUST, lt, false);
  } else if (is_curly_list(e)) {
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      build_ud_rhs(ei, stmt, BasicVar::Var_MUST, lt, false);
    }
  } else if (TYPEOF(e) == LANGSXP) {   // regular function call
    ei->insert_call_site(cell);
    if (is_var(CAR(e))) {
      make_use_var(ei, e, UseVar::UseVar_FUNCTION, BasicVar::Var_MUST, lt);
    } else {
      build_ud_rhs(ei, e, BasicVar::Var_MUST, lt, false);
    }
    // recur on args
    // Because R functions are call-by-need, var uses that appear in
    // args passed to them are MAY-use. BUILTINSXP functions are CBV,
    // so vars passed as args should stay MUST.
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      if (is_var(CAR(e)) && TYPEOF(Rf_findFunUnboundOK(CAR(e), R_GlobalEnv, TRUE)) == BUILTINSXP) {
	build_ud_rhs(ei, stmt, BasicVar::Var_MUST, lt, false);
      } else {
	build_ud_rhs(ei, stmt, BasicVar::Var_MAY, lt, false);
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
/// lt            = whether to initialize as TOP or NONE
/// lhs_type      = whether we are in a local or free assignment
void SexpTraversal::build_ud_lhs(ExpressionInfo * ei,
				 const SEXP cell,
				 const SEXP rhs_c,
				 BasicVar::MayMustT may_must_type,
				 Locality::LocalityType lt,
				 LhsType lhs_type)
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
    if (lt == Locality::Locality_NONE) {
      locality = Locality::Locality_NONE;
    } else if (lhs_type == IN_LOCAL_ASSIGN) {
      locality = Locality::Locality_LOCAL;
    } else {
      locality = Locality::Locality_FREE;
    }
    make_def_var(ei, cell, DefVar::DefVar_ASSIGN, may_must_type, locality, rhs_c);
  } else if (is_struct_field(e)) {
    build_ud_lhs(ei, struct_field_lhs_c(e), rhs_c, BasicVar::Var_MAY, lt, lhs_type);
  } else if (is_subscript(e)) {
    build_ud_lhs(ei, subscript_lhs_c(e), rhs_c, BasicVar::Var_MAY, lt, lhs_type);
    for (SEXP sub_c = subscript_first_sub_c(e); sub_c != R_NilValue; sub_c = CDR(sub_c)) {    
      build_ud_rhs(ei, sub_c, BasicVar::Var_MUST, lt, false);
    }
  } else if (TYPEOF(e) == LANGSXP) {  // regular function call
    // Function application as lvalue. Examples: dim(x) <- foo, attr(x, "dim") <- c(2, 5)
    //
    // TODO: We should really be checking if the function is valid;
    // only some functions applied to arguments make a valid lvalue.
    build_ud_rhs(ei, e, BasicVar::Var_MUST, lt, false);                 // assignment function
    build_ud_lhs(ei, CDR(e), rhs_c, BasicVar::Var_MAY, lt, lhs_type);   // first arg is lvalue
    SEXP arg = CDDR(e);                                    // other args are rvalues if they exist
    while (arg != R_NilValue) {
      build_ud_rhs(ei, arg, BasicVar::Var_MUST, lt, false);
      arg = CDR(arg);
    }
  } else {
    assert(0);
  }
}

void SexpTraversal::make_use_var(ExpressionInfo * ei,
				 SEXP cell,
				 UseVar::PositionT pos,
				 BasicVar::MayMustT mmt,
				 Locality::LocalityType lt)
{
  UseVar * use = new UseVar(cell, pos, mmt, lt);
  putProperty(BasicVar, cell, use);
  ei->insert_use(cell);
}

void SexpTraversal::make_def_var(ExpressionInfo * ei,
				 SEXP cell,
				 DefVar::SourceT source,
				 BasicVar::MayMustT mmt,
				 Locality::LocalityType lt,
				 SEXP rhs_c)
{
  DefVar * def = new DefVar(cell, source, mmt, lt, rhs_c);
  putProperty(BasicVar, cell, def);
  ei->insert_def(cell);
}

SexpTraversal * SexpTraversal::s_instance = 0;

}  // End namespace RAnnot
