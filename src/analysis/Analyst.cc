#include <algorithm>

#include <analysis/Analyst.h>


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
				     RScopeTree::iterator &curr)
{
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
    if (is_simple_assign(e)) {            // a variable bound to a function
      RScopeTree::iterator newfun;
      SEXP var = CAR(assign_lhs_c(e));
      SEXP rhs = CAR(assign_rhs_c(e));
      if (is_fundef(rhs)) {
	newfun = t->append_child(curr, new RFunctionScopeInfo(var, rhs));

	// now skip to body of function to prevent a later pass from
	// finding the function definition; we don't want it to be
	// flagged as a duplicate "anonymous" function.
	build_scope_tree_rec(CAR(fundef_body_c(rhs)), t, newfun);
      }
    } else if (is_fundef(e)) {  // anonymous function
      RScopeTree::iterator newfun = t->append_child(curr, new RFunctionScopeInfo(R_NilValue, e));
      build_scope_tree_rec(CAR(fundef_body_c(e)), t, newfun);
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
// R_BodyVarRef methods
//--------------------------------------------------------------------

// Comparison operators for use in STL containers

bool R_BodyVarRef::operator<(R_VarRef & loc2) const {
  return (get_name() < loc2.get_name());
}

bool R_BodyVarRef::operator<(const R_VarRef & loc2) const {
  // FIXME: find that chapter on how to const cast to get rid of repeated code
  return (get_name() < loc2.get_name());
}

//! Do the locations refer to the same name?
bool R_BodyVarRef::operator==(R_VarRef & loc2) const {
  return (get_name() == loc2.get_name());
}

//! Are they the same location?
bool R_BodyVarRef::equiv(R_VarRef & loc2) const {
  return (m_loc == loc2.get_sexp());
}

std::string R_BodyVarRef::toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const {
  return toString();
}

std::string R_BodyVarRef::toString() const {
  return std::string(CHAR(PRINTNAME(CAR(m_loc))));
}

SEXP R_BodyVarRef::get_sexp() const {
  return m_loc;
}

SEXP R_BodyVarRef::get_name() const {
  return CAR(m_loc);
}

//--------------------------------------------------------------------
// R_ArgVarRef methods
//--------------------------------------------------------------------

// Comparison operators for use in STL containers

bool R_ArgVarRef::operator<(R_VarRef & loc2) const {
  return (get_name() < loc2.get_name());
}

bool R_ArgVarRef::operator<(const R_VarRef & loc2) const {
  return (get_name() < loc2.get_name());
}

//! Do the locations refer to the same name?
bool R_ArgVarRef::operator==(R_VarRef & loc2) const {
  return (get_name() == loc2.get_name());
}

//! Are they the same location?
bool R_ArgVarRef::equiv(R_VarRef & loc2) const {
  return (m_loc == loc2.get_sexp());
}

std::string R_ArgVarRef::toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir) const {
  return toString();
}

std::string R_ArgVarRef::toString() const {
  return std::string(CHAR(PRINTNAME(TAG(m_loc))));
}

SEXP R_ArgVarRef::get_sexp() const {
  return m_loc;
}

SEXP R_ArgVarRef::get_name() const {
  return TAG(m_loc);
}

OA::OA_ptr<R_VarRefSet> refs_from_arglist(SEXP arglist) {
  OA::OA_ptr<R_VarRefSet> refs; refs = new R_VarRefSet;
  R_ListIterator iter(arglist);
  for( ; iter.isValid(); ++iter) {
    OA::OA_ptr<R_ArgVarRef> arg;
    arg = new R_ArgVarRef(iter.current());
    refs->insert_arg(arg);
  }
  return refs;
}


//--------------------------------------------------------------------
// R_ExpUDInfo methods
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

//--------------------------------------------------------------------
// R_VarRefSet methods
//--------------------------------------------------------------------

void R_VarRefSet::insert_ref(const OA::OA_ptr<R_BodyVarRef> var) {
  vars->insert(var);
}

void R_VarRefSet::insert_arg(const OA::OA_ptr<R_ArgVarRef> var) {
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

OA::OA_ptr<R_VarRefSetIterator> R_VarRefSet::get_iterator() const {
  OA::OA_ptr<R_VarRefSetIterator> it;
  it = new R_VarRefSetIterator(vars);
  return it;
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
