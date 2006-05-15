// -*- Mode: C++ -*-

// Set of VarBinding annotations that describe the binding scopes of
// variables.

#include <vector>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/Annotation.h>
#include <analysis/HandleInterface.h>
#include <analysis/PropertySet.h>
#include <analysis/VarBinding.h>
#include <analysis/VarBindingAnnotationMap.h>

#include "VarBindingAnnotationMap.h"

using namespace RAnnot;

// ----- forward declarations of file-scope functions

static bool defined_local_in_scope(Var * v, FuncInfo * s);

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

// ----- singleton pattern -----

VarBindingAnnotationMap * VarBindingAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT VarBindingAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void VarBindingAnnotationMap::create() {
  m_instance = new VarBindingAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

VarBindingAnnotationMap * VarBindingAnnotationMap::m_instance = 0;
PropertyHndlT VarBindingAnnotationMap::m_handle = "VarBinding";

// ----- demand-driven analysis -----

MyMappedT & VarBindingAnnotationMap::operator[](const MyKeyT & k) {
  rcc_error("VarBindingAnnotationMap::operator[] not implemented");
}

MyMappedT VarBindingAnnotationMap::get(const MyKeyT & k) {
  if (!m_computed) {
    compute();
    m_computed = true;
  }

  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    rcc_error("Possible invalid key not found in VarBinding map");
  }

  return annot->second;
}

bool VarBindingAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator VarBindingAnnotationMap::begin() { return m_map.begin(); }
iterator VarBindingAnnotationMap::end() { return m_map.end(); }
const_iterator VarBindingAnnotationMap::begin() const { return m_map.begin(); }
const_iterator VarBindingAnnotationMap::end() const { return m_map.end(); }

// ----- computation -----

/// Each Var is bound in some subset of the sequence of ancestor
/// scopes of its containing function. Compute the subsequence of
/// scopes in which each Var is bound and add the (Var,sequence) pair
/// into the annotation map.
void VarBindingAnnotationMap::compute() {
  static FuncInfo * global = R_Analyst::get_instance()->get_scope_tree_root();
  // for each scope
  FuncInfoIterator fii(global);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo * fi = fii.Current();
    assert(fi != 0);
    // each mention
    FuncInfo::mention_iterator mi;
    for (mi = fi->beginMentions(); mi != fi->endMentions(); ++mi) {
      Var * v = *mi;
      v = getProperty(Var, v->getMention_c()); 
      // FIXME: should make sure we always get the data-flow-solved
      // version of the Var. Shouldn't have to loop through
      // getProperty!
      VarBinding * scopes = new VarBinding();
      if (fi == global) {
	scopes->insert(global);
      } else {
	bool binding_found;
	switch(v->getScopeType()) {
	case Var::Var_LOCAL:
	  // bound in current scope only
	  scopes->insert(fi);
	  break;
	case Var::Var_INDEFINITE:
	  // bound in current scope and one or more ancestors
	  scopes->insert(fi);
	  // FALLTHROUGH
	case Var::Var_FREE:
	  // start at this scope's parent; iterate upward through ancestors
	  binding_found = false;
	  for(FuncInfo * a = fi->Parent(); a != 0; a = a->Parent()) {
	    if (defined_local_in_scope(v,a)) {
	      scopes->insert(a);
	      binding_found = true;
	    }
	  }
	  // If no definition found at any ancestor scope, binding is global.
	  // This is the case for library function uses, etc.
	  if (!binding_found) {
	    scopes->insert(global);
	  }
	  break;
	default:
	  rcc_error("Unknown scope type encountered");
	  break;
	}
      }
      // whether global or not...
      m_map[HandleInterface::make_sym_h(v->getMention_c())] = scopes;
    }
  }
}
  
  
}

/// Is the given variable defined as local in the given scope?
static bool defined_local_in_scope(Var * v, FuncInfo * s) {
  FuncInfo::mention_iterator mi;
  for(mi = s->beginMentions(); mi != s->endMentions(); ++mi) {
    Var * m = *mi;
    if (m->getUseDefType() == Var::Var_DEF  &&
	m->getScopeType() == Var::Var_LOCAL &&
	m->getName() == v->getName())
    {
      return true;
    }
  }
  return false;
}
