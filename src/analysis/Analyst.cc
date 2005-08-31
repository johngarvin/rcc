#include <algorithm>

#include <analysis/Annotation.h>
#include <analysis/Utils.h>
#include <analysis/SimpleIterators.h>

#include "Analyst.h"

#include <analysis/AnalysisResults.h>

using namespace RAnnot;
using namespace RProp;

FuncInfo *R_Analyst::get_scope_tree() {
  if (m_scope_tree == NULL) {
    m_scope_tree = new FuncInfo((FuncInfo*) NULL, R_NilValue, R_NilValue);
    build_scope_tree_rec(m_program, m_scope_tree);
  }
  return m_scope_tree;
}

//! Recursively traverse e to build the scope tree. Each new function
//! definition found is added to the tree t. The iterator curr points to
//! the current function definition.
void R_Analyst::build_scope_tree_rec(SEXP e, FuncInfo *parent) {
  switch(TYPEOF(e)) {
  case NILSXP:
  case REALSXP:
  case STRSXP:
  case LGLSXP:
  case SYMSXP:
    return;
    break;
  case LISTSXP:
    build_scope_tree_rec(CAR(e), parent);
    build_scope_tree_rec(CDR(e), parent);
    break;
  case LANGSXP:
    if (is_simple_assign(e)) {            // a variable bound to a function
      SEXP var = CAR(assign_lhs_c(e));
      SEXP rhs = CAR(assign_rhs_c(e));
      if (is_fundef(rhs)) {
	FuncInfo *newfun = new FuncInfo(parent, var, rhs);
     	putProperty(FuncInfo, rhs, newfun);

	// now skip to body of function to prevent a later pass from
	// finding the function definition; we don't want it to be
	// flagged as a duplicate "anonymous" function.
	build_scope_tree_rec(CAR(fundef_body_c(rhs)),  newfun);
      }
    } else if (is_fundef(e)) {  // anonymous function
      FuncInfo *newfun = new FuncInfo(parent, R_NilValue, e);
      putProperty(FuncInfo, e, newfun);
      build_scope_tree_rec(CAR(fundef_body_c(e)), newfun);
    } else {                   // ordinary function call
      build_scope_tree_rec(CAR(e), parent);
      build_scope_tree_rec(CDR(e), parent);
    }
    break;
  default:
    assert(0);
  }
}

void R_Analyst::dump_cfg(std::ostream &os, SEXP proc) {
#if 0
  OA::ProcHandle ph((OA::irhandle_t)proc);
  //  if (m_cfgs == NULL) {
  //    build cfgs
  //  }
  AnnotationSet::iterator cfg_p; cfg_p = m_cfgs.find(ph);
  if (cfg_p == m_cfgs.end()) { // not found
    // ignore nonexistent CFG
  } else {
    cfg_p->second->dump(os);
  }
#endif
}

//! Populate m_cfgs with the CFG for each procedure
void R_Analyst::build_cfgs() {
  const RScopeTree &t = get_scope_tree();

  OA::OA_ptr<RAnnot::AnnotationSet> aset;
  RScopeTree::iterator scope_it;

  // build CFG for each function
  OA::CFG::ManagerStandard cfg_man(m_interface, true); // build a statement-level CFG
  for(scope_it = t.begin(); scope_it != t.end(); ++scope_it) {
    SEXP fundef = (*scope_it)->get_defn();
    OA::ProcHandle ph((OA::irhandle_t)fundef);
    OA::OA_ptr<OA::CFG::Interface> cfg_ptr; cfg_ptr = cfg_man.performAnalysis(ph);
    get_func_info(ph)->setCFG(cfg_ptr);
  }
}

RAnnot::FuncInfo *R_Analyst::get_func_info(OA::ProcHandle ph) {
  return NULL;  // m_func_info[ph]; -- as soon as AnnotationSet is a map instead of multimap
}

#if 0

//--------------------------------------------------------------------
// R_ExpUDInfo methods -- R_ExpUDInfo is obsolete
//--------------------------------------------------------------------

//! build the sets of uses and defs, non-lvalue (right-hand side of
//! assignment, or non-assignment expression) Uses and defs must be
//! mutually exclusive for different statements--that is, in a loop or
//! if statement, we count only the variables used/defined in the
//! conditional expression, not those in the body or the true or false
//! clauses.

void R_ExpUDInfo::build_ud_rhs(const SEXP e) {
  switch (TYPEOF(e)) {
  case NILSXP:
  case REALSXP:
  case STRSXP:
  case LGLSXP:
    // ignore
    break;
  case SYMSXP:
    non_app_uses->insert(e);
    break;
  case LISTSXP:
    build_ud_rhs(CAR(e));
    build_ud_rhs(CDR(e));
    break;
  case LANGSXP:
    if (is_local_assign(e)) {
      build_ud_lhs(CADR(e), LOCAL);
      build_ud_rhs(CADDR(e));
    } else if (is_free_assign(e)) {
      build_ud_lhs(CADR(e), FREE);
      build_ud_rhs(CADDR(e));
    } else if (is_fundef(e)) {
      // ignore
    } else if (is_struct_field(e)) {
      build_ud_rhs(CADR(e));
    } else if (is_subscript(e)) {
      build_ud_rhs(CADR(e));
      build_ud_rhs(CADDR(e));
    } else {                         // ordinary function application
      if (TYPEOF(CAR(e)) == SYMSXP) {
	app_uses->insert(CAR(e));
      } else {
	build_ud_rhs(CAR(e));
      }
      build_ud_rhs(CDR(e));
    }
    break;
  default:
    assert(0);
    break;
  }
}

//! Build the sets of uses and defs for an lvalue. The predicate
//! is_local is LOCAL if we're in a local ("<-" or "=") assignment or
//! FREE if we're in a free ("<<-") assignment.
//! Uses and defs must be mutually exclusive for different
//! statements--that is, in a compound statement only the variables
//! directly used or modified count, not those found in the loop body.
void R_ExpUDInfo::build_ud_lhs(const SEXP e, local_pred is_local) {
  switch(TYPEOF(e)) {
  case SYMSXP:
    if (is_local == LOCAL) {
      local_defs->insert(e);
    } else if (is_local == FREE) {
      free_defs->insert(e);
    }
    break;
  case LANGSXP:
    if (is_subscript(e)) {
      build_ud_lhs(CADR(e), is_local);
      build_ud_rhs(CADDR(e));
    } else if (is_struct_field(e)) {
      build_ud_lhs(CADR(e), is_local);
    } else if (is_fundef(e)) {
      // ignore
    } else {
      // Application as lvalue. For example: dim(x) <- foo
      //
      // FIXME: We should really be checking if the function is valid;
      // only some functions applied to arguments make a valid lvalue.
      build_ud_lhs(CADR(e), is_local);
    }
    break;
  default:
    assert(0);
    break;
  }
}

#endif

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

#if 0

//--------------------------------------------------------------------
// VarSet methods -- VarSet is obsolete
//--------------------------------------------------------------------

//! Insert the variable into the set
void VarSet::insert(const SEXP var) {
  assert (TYPEOF(var) == SYMSXP);
  vars->insert(var);
}

//! Insert each member of set2 into our set
void VarSet::set_union(const VarSet & set2) {
  OA::OA_ptr<VarSetIterator> it; it = set2.get_iterator();
  for ( ; it->isValid(); ++*it) {
    vars->insert(it->current());
  }
}

//! Insert each member of set2 into our set
void VarSet::set_union(OA::OA_ptr<VarSet> set2) {
  OA::OA_ptr<VarSetIterator> it; it = set2->get_iterator();
  for ( ; it->isValid(); ++*it) {
    vars->insert(it->current());
  }
}

#if 0
//! Insert each member of set2 into our set
void VarSet::set_union(OA::OA_ptr<std::set<R_VarRef> > set2) {
  std::set<R_VarRef>::iterator it;
  for (it = set2->begin(); it != set2->end(); ++it) {
    vars->insert(it->get_sexp());
  }
}
#endif

void VarSet::dump() {
  OA::OA_ptr<VarSetIterator> it; it = get_iterator();
  for ( ; it->isValid(); ++*it) {
    std::cout << CHAR(PRINTNAME(it->current())) << std::endl;
  }
}

#endif
