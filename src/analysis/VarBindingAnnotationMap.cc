// -*- Mode: C++ -*-

// Set of VarBinding annotations that describe the binding scopes of
// variables.

#include <analysis/Analyst.h>
#include <analysis/Annotation.h>
#include <analysis/VarBindingAnnotationMap.h>

#include "VarBindingAnnotationMap.h"

namespace RAnnot {

// ----- typedefs for readability -----

typedef VarBindingAnnotationMap::MyKeyT MyKeyT;
typedef VarBindingAnnotationMap::MyMappedT MyMappedT;
typedef VarBindingAnnotationMap::iterator iterator;
typedef VarBindingAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor -----

VarBindingAnnotationMap::VarBindingAnnotationMap(bool ownsAnnotations /* = true */)
  : m_computed(false),
    m_map()
  {}

VarBindingAnnotationMap::~VarBindingAnnotationMap()
  {}

// ----- demand-driven analysis -----

MyMappedT VarBindingAnnotationMap::get(const MyKeyT & k) {
  if (!m_computed) {
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

bool VarBindingAnnotationMap::is_computed() {
  return m_computed;
}

/// Each Var is bound in some subset of the sequence of ancestor
/// scopes of its containing function. Compute the subsequence of
/// scopes in which each Var is bound and add the (Var,sequence) pair
/// into the annotation map.
void VarBindingAnnotationMap::compute() {
#if 0
  // for each scope
  FuncInfoIterator fii(R_Analyst::get_instance()->get_scope_tree_root());
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
	for(FuncInfo * a = fi->Parent(); a != 0; a = a->Parent()) {
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
#endif
}

}
