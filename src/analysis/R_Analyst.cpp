#include "R_Analyst.h"
#include <algorithm>

// TODO: move these to separate file for everyone to access
// static bool is_local_assign(SEXP e);
// static bool is_free_assign(SEXP e);
// static bool is_assign(SEXP e);
// static bool is_simple_assign(SEXP e);
// static bool is_fundef(SEXP e);
// static bool is_struct_field(SEXP e);
// static bool is_subscript(SEXP e);

//! If necessary, analyze the SEXP exp to find the scope tree. In any
//! case, return a pointer to the generated tree. TODO: create a
//! datatype for the scope tree to hide the implementation, and to have
//! an iterator that matches the others.
OA::OA_ptr<RScopeTree> R_Analyst::get_scope_tree() {
  if (scope_tree.ptrEqual(NULL)) {
    scope_tree = new RScopeTree();
    RScopeTree::iterator top = scope_tree->begin();
    RScopeTree::iterator first = scope_tree->insert(top, new RFunctionScopeInfo(R_NilValue, R_NilValue));
    build_scope_tree_rec(exp, scope_tree, first);
  }
  return scope_tree;
}

//! Recursively traverse e to build the scope tree. Each new function
//! definition found is added to the tree t. The iterator curr points to
//! the current function definition.
void R_Analyst::build_scope_tree_rec(SEXP e,
				     OA::OA_ptr<RScopeTree> t,
				     RScopeTree::iterator &curr) {
  switch(TYPEOF(e)) {
  case NILSXP:
  case REALSXP:
  case STRSXP:
  case LGLSXP:
  case SYMSXP:
    return;
    break;
  case LISTSXP:
    build_scope_tree_rec(CAR(e), t, curr);
    build_scope_tree_rec(CDR(e), t, curr);
    break;
  case LANGSXP:
    if (is_simple_assign(e)            // if binding a variable to a function
	// CADDR = RHS of assignment
	&& TYPEOF(CADDR(e)) == LANGSXP
	&& is_fundef(CADDR(e))) {
      RScopeTree::iterator newfun = t->append_child(curr, new RFunctionScopeInfo(CAR(assign_lhs_c(e)), CADDR(e)));

      // now skip to body of function to prevent a later pass from
      // finding the function definition; we don't want it to be
      // flagged as a duplicate "anonymous" function.
      build_scope_tree_rec(CADDR(CADDR(e)), t, newfun);
    } else if (is_fundef(e)) {  // anonymous function
      RScopeTree::iterator newfun = t->append_child(curr, new RFunctionScopeInfo(R_NilValue, e));
      build_scope_tree_rec(CADDR(e), t, newfun);
    } else {                   // ordinary function call
      build_scope_tree_rec(CAR(e), t, curr);
      build_scope_tree_rec(CDR(e), t, curr);
    }
    break;
  default:
    assert(0);
  }
}

//--------------------------------------------------------------------
// R_VarRef methods
//--------------------------------------------------------------------

bool R_VarRef::operator<(R_VarRef & loc2) const {
  return (CAR(m_loc) < CAR(loc2.m_loc));
}

bool R_VarRef::operator<(const R_VarRef & loc2) const {
  return (CAR(m_loc) < CAR(loc2.m_loc));
}

//! Do the locations refer to the same name?
bool R_VarRef::operator==(R_VarRef & loc2) const {
  return (CAR(m_loc) == CAR(loc2.m_loc));
}

//! Are they the same location?
bool R_VarRef::equiv(R_VarRef & loc2) const {
  return (m_loc == loc2.m_loc);
}

std::string R_VarRef::toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const {
  return toString();
}

std::string R_VarRef::toString() const {
  return std::string(CHAR(PRINTNAME(CAR(m_loc))));
}


//--------------------------------------------------------------------
// R_ExpUDInfo methods
//--------------------------------------------------------------------

#if 0

// defunct; now we build uses and defs together

// build list of uses, LHS of assignment statement
void build_uses_lhs(SEXP e)
{
  switch(TYPEOF(e)) {
  case SYMSXP:
    break;
  case LANGSXP:
    if (is_subscript(e)) {
      build_uses_lhs(CADR(e));
      build_uses_rhs(CADDR(e));
    } else if (is_struct_field(e)) {
      build_uses_lhs(CADR(e));
    }
    break;
  default:
    assert(0);
  }
}

// build list of uses, RHS of assignment statement
void build_uses_rhs(SEXP e)
{
  switch(TYPEOF(e)) {
  case SYMSXP:
    // current var is a use
    non_app_uses.push_back(e);
    break;
  case REALSXP:
  case NILSXP:
    ; // do nothing
    break;
  case LISTSXP:
    build_uses_rhs(CAR(e));
    build_uses_rhs(CDR(e));
    break;
  case LANGSXP:
    if (is_assign(e)) {
      build_uses_lhs(CADR(e));
      build_uses_rhs(CADDR(e));
    } else if (is_fundef(e)) {
      ; // ignore
    } else if (is_struct_field(e)) {
      build_uses_rhs(CADR(e));
    } else {  // ordinary function application
      switch(TYPEOF(CAR(e))) {
      case SYMSXP:
	app_uses.push_back(CAR(e));
	break;
      default:
	build_uses_rhs(CAR(e));
	break;
      }
      build_uses_rhs(CDR(e));
    }
    break;
  default:
    assert(0);
  }
}

// build list of defs, LHS of a local (single-arrow) assignment
// statement
void build_defs_lhs_local(SEXP e)
{
  switch(TYPEOF(e)) {
  case SYMSXP:
    // current var is a def
    defs_local.push_back(e);
    break;
  case LANGSXP:
    if (is_subscript(e)) {
      build_defs_lhs(CADR(e));
      build_defs_rhs(CADDR(e));
    } else if (is_struct_field(e)) {
      build_defs_lhs(CADR(e));
    }
    break;
  default:
    assert(0);
  }
}

// build list of defs, LHS of a free (double-arrow) assignment
// statement
void build_defs_lhs_free(SEXP e)
{
  switch(TYPEOF(e)) {
  case SYMSXP:
    // current var is a def
    defs_free.push_back(e);
    break;
  case LANGSXP:
    if (is_subscript(e)) {
      build_defs_lhs(CADR(e));
      build_defs_rhs(CADDR(e));
    } else if (is_struct_field(e)) {
      build_defs_lhs(CADR(e));
    }
    break;
  default:
    assert(0);
  }
}


// build list of defs, RHS of assignment statement
void build_defs_rhs(SEXP e)
{
  switch(TYPEOF(e)) {
  case SYMSXP:
  case REALSXP:
  case NILSXP:
    // do nothing
    break;
  case LISTSXP:
    build_defs_rhs(CAR(e));
    build_defs_rhs(CDR(e));
    break;
  case LANGSXP:
    if (is_local_assign(e)) {
	build_defs_lhs_local(CADR(e));
	build_defs_rhs(CADDR(e));
    } else if (is_free_assign(e)) {
	build_defs_lhs_free(CADR(e));
	build_defs_rhs(CADDR(e));
    } else if (is_fundef(e)) {
      ; // ignore
    } else if (is_struct_field(e)) {
      build_defs_rhs(CADR(e));
    } else {
      build_defs_rhs(CAR(e));
      build_defs_rhs(CDR(e));
    }
    break;
  default:
    assert(0);
  }
}
#endif

//! build the sets of uses and defs, non-lvalue (right-hand side of
//! assignment, or non-assignment expression)
//! Uses and defs must be mutually exclusive for different
//! statements--that is, in a compound statement only the variables
//! directly used or modified count, not those found in the loop body.
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
    non_app_uses->insert(cell);
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
      app_uses->insert(e);
    } else {
      build_ud_rhs(e);
    }
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
      local_defs->insert(cell);
    } else {
      free_defs->insert(cell);
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

//--------------------------------------------------------------------
// R_VarRefSet methods
//--------------------------------------------------------------------

void R_VarRefSet::insert(const R_VarRef var) {
  vars->insert(var);
}

void R_VarRefSet::set_union(const R_VarRefSet & set2) {
  OA::OA_ptr<R_VarRefSetIterator> it; it = set2.get_iterator();
  for ( ; it->isValid(); ++*it) {
    vars->insert(it->current());
  }
}

void R_VarRefSet::set_union(OA::OA_ptr<R_VarRefSet> set2) {
  OA::OA_ptr<R_VarRefSetIterator> it; it = set2->get_iterator();
  for ( ; it->isValid(); ++*it) {
    vars->insert(it->current());
  }
}


//--------------------------------------------------------------------
// VarSet methods
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

//! Insert each member of set2 into our set
void VarSet::set_union(OA::OA_ptr<std::set<R_VarRef> > set2) {
  std::set<R_VarRef>::iterator it;
  for (it = set2->begin(); it != set2->end(); ++it) {
    vars->insert(it->get_sexp());
  }
}

void VarSet::dump() {
  OA::OA_ptr<VarSetIterator> it; it = get_iterator();
  for ( ; it->isValid(); ++*it) {
    std::cout << CHAR(PRINTNAME(it->current())) << std::endl;
  }
}

