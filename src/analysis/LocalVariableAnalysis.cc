// -*-Mode: C++;-*-

#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>

#include "LocalVariableAnalysis.h"

using namespace RAnnot;

LocalVariableAnalysis::LocalVariableAnalysis(const SEXP _stmt)
  : m_stmt(_stmt), m_stmt_annot(new ExpressionInfo())
{
  m_stmt_annot->setDefn(m_stmt);
}

//! Traverse the given SEXP and set variable annotations with local
//! syntactic information.
void LocalVariableAnalysis::perform_analysis() {
  build_ud_rhs(m_stmt);
  // adds Var properties, adds Var pointers to m_annot

  // now annotate the original statement with the set of Var annotations
  putProperty(ExpressionInfo, m_stmt, m_stmt_annot, true);
}

//! Traverse the given SEXP (not an lvalue) and set variable
//! annotations with local syntactic information.
//! cell          = a cons cell whose CAR is the expression we're talking about
void LocalVariableAnalysis::build_ud_rhs(const SEXP cell) {
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  if (is_const(e)) {
    // ignore
  } else if (is_var(e)) {
    UseVar * var_annot = new UseVar();
    var_annot->setMention(cell);
    var_annot->setPositionType(UseVar::UseVar_ARGUMENT);
    var_annot->setMayMustType(Var::Var_MUST);
    var_annot->setScopeType(Var::Var_TOP);
    putProperty(Var, cell, var_annot, true);
    m_stmt_annot->insert_var(var_annot);
  } else if (is_local_assign(e)) {
    build_ud_lhs(assign_lhs_c(e), assign_rhs_c(e), Var::Var_MUST, IN_LOCAL_ASSIGN);
    build_ud_rhs(assign_rhs_c(e));
  } else if (is_free_assign(e)) {
    build_ud_lhs(assign_lhs_c(e), assign_rhs_c(e), Var::Var_MUST, IN_FREE_ASSIGN);
    build_ud_rhs(assign_rhs_c(e));
  } else if (is_fundef(e)) {
    // ignore
  } else if (is_struct_field(e)) {
    build_ud_rhs(CDR(e));
  } else if (is_subscript(e)) {
    build_ud_rhs(subscript_lhs_c(e));
    build_ud_rhs(subscript_rhs_c(e));
  } else if (is_if(e)) {
    build_ud_rhs(if_cond_c(e));
  } else if (is_for(e)) {
    // defines the induction variable, uses the range
    build_ud_lhs(for_iv_c(e), for_range_c(e), Var::Var_MUST, IN_LOCAL_ASSIGN);
    build_ud_rhs(for_range_c(e));
  } else if (is_loop_header(e)) {
    // XXXXX
  } else if (is_loop_increment(e)) {
    // XXXXX
  } else if (is_while(e)) {
    build_ud_rhs(while_cond_c(e));
  } else if (is_repeat(e)) {
    // ignore
  } else if (is_paren_exp(e)) {
    build_ud_rhs(paren_body_c(e));
  } else if (is_curly_list(e)) {
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      build_ud_rhs(stmt);
    }
  } else if (TYPEOF(e) == LANGSXP) {   // regular function call
    if (is_var(CAR(e))) {
      UseVar * var_annot = new UseVar();
      var_annot->setMention(e);
      var_annot->setPositionType(UseVar::UseVar_FUNCTION);
      var_annot->setMayMustType(Var::Var_MUST);
      var_annot->setScopeType(Var::Var_TOP);
      putProperty(Var, e, var_annot, true);
      m_stmt_annot->insert_var(var_annot);
    } else {
      build_ud_rhs(e);
    }
    // recur on args
    for (SEXP stmt = CDR(e); stmt != R_NilValue; stmt = CDR(stmt)) {
      build_ud_rhs(stmt);
    }
  } else {
    assert(0);
  }  
}

//! Traverse the SEXP contained in the given cons cell, assuming that
//! it's an lvalue. Set variable annotations with local syntactic
//! information.
//! cell          = a cons cell whose CAR is the lvalue we're talking about
//! rhs           = a cons cell whose CAR is the right side of the assignment statement
//! may_must_type = whether we are in a may-def or a must-def
//! lhs_type      = whether we are in a local or free assignment
void LocalVariableAnalysis::build_ud_lhs(const SEXP cell, const SEXP rhs,
					 Var::MayMustT may_must_type, LhsType lhs_type)
{
  assert(is_cons(cell));
  assert(is_cons(rhs));
  SEXP e = CAR(cell);
  if (is_var(e)) {
    DefVar * var_annot = new DefVar();
    var_annot->setMention(cell);
    var_annot->setMayMustType(may_must_type);
    var_annot->setSourceType(DefVar::DefVar_ASSIGN);
    var_annot->setRhs(rhs);
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
	var_annot->setScopeType(Var::Var_LOCAL);
      } else {
	var_annot->setScopeType(Var::Var_FREE);
      }
    } else {                                             // may-def
      var_annot->setScopeType(Var::Var_TOP);
    }
    putProperty(Var, cell, var_annot, true);
    m_stmt_annot->insert_var(var_annot);
  } else if (is_struct_field(e)) {
    build_ud_lhs(struct_field_lhs_c(e), rhs, Var::Var_MAY, lhs_type);
  } else if (is_subscript(e)) {
    build_ud_lhs(subscript_lhs_c(e), rhs, Var::Var_MAY, lhs_type);
    build_ud_rhs(subscript_rhs_c(e));
  } else if (TYPEOF(e) == LANGSXP) {  // regular function call
    // Function application as lvalue. For example: dim(x) <- foo
    //
    // FIXME: We should really be checking if the function is valid;
    // only some functions applied to arguments make a valid lvalue.
    assert(CDDR(e) == R_NilValue); // more than one argument is an error, right?
    build_ud_rhs(e);
    build_ud_lhs(CDR(e), rhs, Var::Var_MAY, lhs_type);
  } else {
    assert(0);
  }
}
