// -*- Mode: C++ -*-

// Set of VarBinding annotations that describe the binding scopes of
// variables.

#include <analysis/VarBindingAnnotationMap.h>

namespace RAnnot {

// forward declaration of static functions

static void compute_bound_scopes();


// constructor/destructor

VarBindingAnnotationMap::VarBindingAnnotationMap(bool ownsAnnotations /* = true */) {}

VarBindingAnnotationMap::~VarBindingAnnotationMap() {
  if (m_computed) delete m_ba;
}

void VarBindingAnnotationMap::compute_if_needed() {
  if (!m_computed) {
    m_computed = true;
    compute_bound_scopes();
  }
}


// Each Var is bound in some subset of the sequence of ancestor scopes
// of its containing function. Compute the subsequence of scopes in
// which each Var is bound and add the (Var,sequence) pair into the
// annotation set.
void compute_bound_scopes() {
  // for each scope
  FuncInfoIterator fii(Analyst::getRoot(), PreOrder); // order is arbitrary
  for( ; fii.IsValid(); ++fii) {
    FuncInfo * fi = fii.Current();
    // each mention
    FuncInfo::mention_iterator mi;
    for (mi = fi->beginMentions(); mi != fi->endMentions(); ++mi) {
      Var * v = *mi;
      
      bool binding_found;
      switch(v->getScopeType()) {
      case Var::Var_LOCAL:
	// bound in current scope only
	m_scopes.push_back(v->getContainingScope());
	break;
      case Var::Var_INDEFINITE:
	// bound in current scope and one or more ancestors
	m_scopes.push_back(v->getContainingScope());
	// FALLTHROUGH
      case Var::Var_FREE:
	// start at this scope's parent; iterate upward through ancestors
	binding_found = false;
	for(FuncInfo* a = fi->Parent(); a != 0; a = a->Parent()) {
	  if (defined_local_in_scope(v,a)) {
	    m_scopes.push_back(a);
	    binding_found = true;
	  }
	}
	// If no definition found at any ancestor scope, binding is global.
	// This is the case for library function uses, etc.
	if (!binding_found) {
	  m_scopes.push_back(FuncInfo::getGlobalScope());
	}
	break;
      default:
	rcc_error("Unknown scope type encountered");
	break;
      }
    }
  }
}

}
