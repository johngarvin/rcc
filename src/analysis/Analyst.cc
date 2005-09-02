#include <algorithm>

#include <analysis/Annotation.h>
#include <analysis/Utils.h>
#include <analysis/SimpleIterators.h>
#include <analysis/AnalysisResults.h>
#include <analysis/HandleInterface.h>
#include <analysis/UseDefSolver.h>

#include "Analyst.h"

using namespace RAnnot;
using namespace RProp;

//! construct an R_Analyst by providing an SEXP representing the whole program
R_Analyst::R_Analyst(SEXP _program) : m_program(_program) {
  m_interface = new R_IRInterface();
  m_scope_tree_root = new FuncInfo((FuncInfo*) NULL, R_NilValue, R_NilValue);
  build_scope_tree(m_program, m_scope_tree_root);
  build_cfgs();
  //  build_use_def_info();
}

FuncInfo *R_Analyst::get_scope_tree_root() {
  return m_scope_tree_root;
}

//! Recursively traverse e to build the scope tree. 'parent' is a
//! pointer to the parent lexical scope.
void R_Analyst::build_scope_tree(SEXP e, FuncInfo *parent) {
  switch(TYPEOF(e)) {
  case NILSXP:
  case REALSXP:
  case STRSXP:
  case LGLSXP:
  case SYMSXP:
    return;
    break;
  case LISTSXP:
    build_scope_tree(CAR(e), parent);
    build_scope_tree(CDR(e), parent);
    break;
  case LANGSXP:
    if (is_simple_assign(e)) {            // a variable bound to a function
      SEXP var = CAR(assign_lhs_c(e));
      SEXP rhs = CAR(assign_rhs_c(e));
      if (is_fundef(rhs)) {
	FuncInfo *newfun = new FuncInfo(parent, var, rhs);
     	putProperty(FuncInfo, rhs, newfun, false);

	// now skip to body of function to prevent a later pass from
	// finding the function definition; we don't want it to be
	// flagged as a duplicate "anonymous" function.
	build_scope_tree(CAR(fundef_body_c(rhs)), newfun);
      }
    } else if (is_fundef(e)) {  // anonymous function
      FuncInfo *newfun = new FuncInfo(parent, R_NilValue, e);
      putProperty(FuncInfo, e, newfun, false);
      build_scope_tree(CAR(fundef_body_c(e)), newfun);
    } else {                   // ordinary function call
      build_scope_tree(CAR(e), parent);
      build_scope_tree(CDR(e), parent);
    }
    break;
  default:
    assert(0);
  }
}

void R_Analyst::dump_cfg(std::ostream &os, SEXP proc) {
  if (proc != R_NilValue) {
    FuncInfo *fi = getProperty(FuncInfo, proc);
    OA::OA_ptr<OA::CFG::Interface> cfg;
    cfg = fi->getCFG();
    cfg->dump(os, m_interface);
  }
}

void R_Analyst::dump_all_cfgs(std::ostream &os) {
  FuncInfoIterator fii(m_scope_tree_root);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo *finfo = fii.Current();
    OA::OA_ptr<OA::CFG::Interface> cfg;
    cfg = finfo->getCFG();
    if (!cfg.ptrEqual(NULL)) {
      cfg->dump(os, m_interface);
    }
  }
}

//! Populate m_cfgs with the CFG for each procedure
void R_Analyst::build_cfgs() {
  FuncInfo *finfo = get_scope_tree_root();
  OA::CFG::ManagerStandard cfg_man(m_interface, true); // build statement-level CFGs

  OA::OA_ptr<RAnnot::AnnotationSet> aset;

  // preorder traversal of scope tree
  FuncInfoIterator fii(finfo);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo *finfo = fii.Current();
    SEXP fundef = finfo->get_defn();
    if (fundef != R_NilValue) {
      OA::ProcHandle ph = HandleInterface::make_proc_h(fundef);
      OA::OA_ptr<OA::CFG::Interface> cfg_ptr; cfg_ptr = cfg_man.performAnalysis(ph);
      finfo->setCFG(cfg_ptr);
    }
  }
}

void R_Analyst::build_use_def_info() {
  FuncInfoIterator fii(m_scope_tree_root);
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    R_UseDefSolver uds(m_interface);
    AnnotationSet *anset = uds.perform_analysis(HandleInterface::make_proc_h(fi->get_defn()), fi->getCFG());
    analysisResults[Var::VarProperty] = anset;
  }
}

//--------------------------------------------------------------------
// R_ExpUDLocInfo methods
//--------------------------------------------------------------------

//! build the sets of uses and defs, non-lvalue (right-hand side of
//! assignment, or non-assignment expression)
//! Uses and defs must be mutually exclusive for different
//! statements--that is, in a compound statement only the variables
//! directly used or modified count, not those found in the loop body.
//! Input is a CONS cell containing the expression as its CAR.
void R_ExpUDLocInfo::build_ud_rhs(const SEXP cell) {
  assert(TYPEOF(cell) == LISTSXP || TYPEOF(cell) == LANGSXP);
  SEXP e = CAR(cell);
  if (is_const(e)) {
    // ignore
  } else if (is_var(e)) {
    OA::OA_ptr<R_BodyVarRef> ref; ref = new R_BodyVarRef(cell);
    non_app_uses->insert_ref(ref);
  } else if (is_local_assign(e)) {
    build_ud_lhs(assign_lhs_c(e), LOCAL);
    build_ud_rhs(assign_rhs_c(e));
  } else if (is_free_assign(e)) {
    build_ud_lhs(assign_lhs_c(e), FREE);
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
    build_ud_lhs(for_iv_c(e), LOCAL);
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
      OA::OA_ptr<R_BodyVarRef> ref; ref = new R_BodyVarRef(e);
      app_uses->insert_ref(ref);
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

//! Build the sets of uses and defs for an lvalue. The predicate
//! is_local is LOCAL if we're in a local ("<-" or "=") assignment or
//! FREE if we're in a free ("<<-") assignment.
//! Uses and defs must be mutually exclusive for different
//! statements--that is, in a compound statement only the variables
//! directly used or modified count, not those found in the loop body.
void R_ExpUDLocInfo::build_ud_lhs(const SEXP cell, local_pred is_local) {
  SEXP e = CAR(cell);
  if (is_var(e)) {
    if (is_local == LOCAL) {
      OA::OA_ptr<R_BodyVarRef> ref; ref = new R_BodyVarRef(cell);
      local_defs->insert_ref(ref);
    } else {
      OA::OA_ptr<R_BodyVarRef> ref; ref = new R_BodyVarRef(cell);
      free_defs->insert_ref(ref);
    }
  } else if (is_struct_field(e)) {
    build_ud_lhs(CDR(e), is_local);
  } else if (is_subscript(e)) {
    build_ud_lhs(subscript_lhs_c(e), is_local);
    build_ud_rhs(subscript_rhs_c(e));
  } else if (TYPEOF(e) == LANGSXP) {  // regular function call
    // Function application as lvalue. For example: dim(x) <- foo
    //
    // FIXME: We should really be checking if the function is valid;
    // only some functions applied to arguments make a valid lvalue.
    assert(CDDR(e) == R_NilValue); // more than one argument is an error, right?
    build_ud_rhs(e);
    build_ud_lhs(CDR(e), is_local);
  } else {
    assert(0);
  }
}
