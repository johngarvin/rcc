#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Assertion.h>

#include "ScopeTreeBuilder.h"

using namespace RAnnot;

/// Build a scope tree rooted at the given fundef assignment.
FuncInfo * ScopeTreeBuilder::build_scope_tree_with_given_root(SEXP r_root) {
  assert(is_simple_assign(r_root));
  SEXP name = CAR(assign_lhs_c(r_root));
  SEXP definition = CAR(assign_rhs_c(r_root));
  assert(is_fundef(definition));
  FuncInfo *root = new FuncInfo(0, name, definition); // root node has null parent
  putProperty(FuncInfo, definition, root, false);

  // Skip to the body of the function. Otherwise, the definition we
  // just recorded would be flagged as a duplicate "anonymous"
  // function.
  build_scope_tree(CAR(fundef_body_c(definition)), root);
  return root;
}

/// Recursively traverse e to build the scope tree. 'parent' is a
/// pointer to the parent lexical scope.
void ScopeTreeBuilder::build_scope_tree(SEXP e, FuncInfo *parent) {
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
    if (is_simple_assign(e)) {
      SEXP var = CAR(assign_lhs_c(e));
      SEXP rhs = CAR(assign_rhs_c(e));
      if (is_fundef(rhs)) {                  // a variable bound to a function
	FuncInfo *newfun = new FuncInfo(parent, var, rhs);
     	putProperty(FuncInfo, rhs, newfun, false);

	// Skip to the body of the function. Otherwise, the definition we
	// just recorded would be flagged as a duplicate "anonymous"
	// function.
	build_scope_tree(CAR(fundef_body_c(rhs)), newfun);
      } else {
	build_scope_tree(rhs, parent);
      }
    } else if (is_fundef(e)) {  // anonymous function
      FuncInfo *newfun = new FuncInfo(parent, R_NilValue, e);
      putProperty(FuncInfo, e, newfun, false);
      build_scope_tree(CAR(fundef_body_c(e)), newfun);
    } else if (is_rcc_assertion(e)) { // rcc_assert call
      FuncInfo *fi;
      if (is_rcc_assert_exp(e)) {
	SEXP body = CADR(e);
	build_scope_tree(body, parent);
	if (is_simple_assign(body)) {
	  SEXP rhs = CAR(assign_rhs_c(body));
	  if (is_fundef(rhs)) fi = getProperty(FuncInfo, rhs);
	}
      } else if (is_rcc_assert(e)) {
	fi = parent;
      }
      process_assert(e, fi);
    } else {                   // ordinary function call
      build_scope_tree(CAR(e), parent);
      build_scope_tree(CDR(e), parent);
    }
    break;
  default:
    assert(0);
  }
}
