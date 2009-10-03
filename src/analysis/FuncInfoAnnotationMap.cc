// -*- Mode: C++ -*-

// Set of FuncInfo annotations representing function information,
// including the methods for computing the information. This should
// probably be split up into the different analyses at some point.

#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/Assertion.h>
#include <analysis/HandleInterface.h>
#include <analysis/Utils.h>

#include "FuncInfoAnnotationMap.h"

using namespace HandleInterface;

namespace RAnnot {

// type definitions for readability
typedef FuncInfoAnnotationMap::MyKeyT MyKeyT;
typedef FuncInfoAnnotationMap::MyMappedT MyMappedT;
typedef FuncInfoAnnotationMap::iterator iterator;
typedef FuncInfoAnnotationMap::const_iterator const_iterator;

//  ----- constructor/destructor ----- 

FuncInfoAnnotationMap::FuncInfoAnnotationMap() {}

FuncInfoAnnotationMap::~FuncInfoAnnotationMap() {
  // This AnnotationMap owns all FuncInfos, so we want to delete them now.
  // FuncInfos are in JMC's tree structure; deleting the root will delete all the FuncInfos.
  delete R_Analyst::get_instance()->get_scope_tree_root();
}

// ----- singleton pattern -----

FuncInfoAnnotationMap * FuncInfoAnnotationMap::get_instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT FuncInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

void FuncInfoAnnotationMap::create() {
  s_instance = new FuncInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

FuncInfoAnnotationMap * FuncInfoAnnotationMap::s_instance = 0;
PropertyHndlT FuncInfoAnnotationMap::s_handle = "FuncInfo";

// ----- computation -----

void FuncInfoAnnotationMap::compute() {
  SEXP r_root = R_Analyst::get_instance()->get_program();
  assert(is_simple_assign(r_root));

  build_scope_tree(r_root);
  for (const_iterator it = begin(); it != end(); ++it) {
    FuncInfo * fn = dynamic_cast<FuncInfo *>(it->second);  // TODO: redo annotation maps to avoid casts
    fn->perform_analysis();
  }
}

void FuncInfoAnnotationMap::build_scope_tree(SEXP r_root) {
  // create the root FuncInfo node
  SEXP name_c = assign_lhs_c(r_root);
  SEXP definition = CAR(assign_rhs_c(r_root));
  assert(is_fundef(definition));
  FuncInfo * a_root = new FuncInfo(0, name_c, definition); // root node has null parent
  get_map()[definition] = a_root;
  
  // Skip to the body of the function. Otherwise, the definition we
  // just recorded would be flagged as a duplicate "anonymous"
  // function.
  build_scope_tree_rec(CAR(fundef_body_c(definition)), a_root);
}

void FuncInfoAnnotationMap::build_scope_tree_rec(SEXP e, FuncInfo * parent) {
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
    if (is_simple_assign(e)) {
      SEXP name_c = assign_lhs_c(e);
      SEXP rhs = CAR(assign_rhs_c(e));
      if (is_fundef(rhs)) {                  // a variable bound to a function
	FuncInfo * newfun = new FuncInfo(parent, name_c, rhs);
	get_map()[rhs] = newfun;

	// Skip to the body of the function. Otherwise, the definition we
	// just recorded would be flagged as a duplicate "anonymous"
	// function.
	build_scope_tree_rec(CAR(fundef_body_c(rhs)), newfun);
      } else {
	build_scope_tree_rec(rhs, parent);
      }
    } else if (is_fundef(e)) {  // anonymous function
      FuncInfo * newfun = new FuncInfo(parent, Rf_cons(Rf_install("<unknown function>"), R_NilValue), e);
      get_map()[e] = newfun;
      build_scope_tree_rec(CAR(fundef_body_c(e)), newfun);
    } else if (is_rcc_assertion(e)) { // rcc_assert call
      // Assertions are processed here.
      FuncInfo * fi;
      if (is_rcc_assert_exp(e)) {
	// rcc.assert.exp assertion: 
	SEXP body = CADR(e);
	build_scope_tree_rec(body, parent);
	if (is_simple_assign(body)) {
	  SEXP rhs = CAR(assign_rhs_c(body));
	  if (is_fundef(rhs)) fi = getProperty(FuncInfo, rhs); // TODO: is this right?
	}
      } else if (is_rcc_assert(e)) {
	fi = parent;
      }
      process_assert(e, fi);
    } else {                   // ordinary function call
      build_scope_tree_rec(CAR(e), parent);
      build_scope_tree_rec(CDR(e), parent);
    }
    break;
  default:
    assert(0);
  }
}

}
