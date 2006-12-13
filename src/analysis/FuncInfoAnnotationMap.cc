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

FuncInfoAnnotationMap::FuncInfoAnnotationMap(bool ownsAnnotations /* = true */)
: m_computed(false),
  m_map()
  {}

FuncInfoAnnotationMap::~FuncInfoAnnotationMap() {}

// ----- singleton pattern -----

FuncInfoAnnotationMap * FuncInfoAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT FuncInfoAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void FuncInfoAnnotationMap::create() {
  m_instance = new FuncInfoAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

FuncInfoAnnotationMap * FuncInfoAnnotationMap::m_instance = 0;
PropertyHndlT FuncInfoAnnotationMap::m_handle = "FuncInfo";

//  ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & FuncInfoAnnotationMap::operator[](const MyKeyT & k) {
  return m_map[k];
}

// Perform the computation if necessary and return the requested data.
MyMappedT FuncInfoAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    rcc_error("Possible invalid key not found in map");
  }

  return annot->second;
}

bool FuncInfoAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator FuncInfoAnnotationMap::begin() { return m_map.begin(); }
const_iterator  FuncInfoAnnotationMap::begin() const { return m_map.begin(); }
iterator  FuncInfoAnnotationMap::end() { return m_map.end(); }
const_iterator  FuncInfoAnnotationMap::end() const { return m_map.end(); }

// ----- computation -----

void FuncInfoAnnotationMap::compute() {
  SEXP r_root = R_Analyst::get_instance()->get_program();
  assert(is_simple_assign(r_root));

  build_scope_tree(r_root);
}

void FuncInfoAnnotationMap::build_scope_tree(SEXP r_root) {
  // create the root FuncInfo node
  SEXP name = CAR(assign_lhs_c(r_root));
  SEXP definition = CAR(assign_rhs_c(r_root));
  assert(is_fundef(definition));
  FuncInfo * a_root = new FuncInfo(0, name, definition); // root node has null parent
  m_map[make_proc_h(definition)] = a_root;
  
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
      SEXP var = CAR(assign_lhs_c(e));
      SEXP rhs = CAR(assign_rhs_c(e));
      if (is_fundef(rhs)) {                  // a variable bound to a function
	FuncInfo * newfun = new FuncInfo(parent, var, rhs);
	m_map[make_proc_h(rhs)] = newfun;

	// Skip to the body of the function. Otherwise, the definition we
	// just recorded would be flagged as a duplicate "anonymous"
	// function.
	build_scope_tree_rec(CAR(fundef_body_c(rhs)), newfun);
      } else {
	build_scope_tree_rec(rhs, parent);
      }
    } else if (is_fundef(e)) {  // anonymous function
      FuncInfo * newfun = new FuncInfo(parent, R_NilValue, e);
      m_map[HandleInterface::make_proc_h(e)] = newfun;
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
	  if (is_fundef(rhs)) fi = getProperty(FuncInfo, rhs); // ?????
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
