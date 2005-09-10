#include "BindingAnalysis.h"

using namespace RAnnot;

// forward declaration

static bool defined_local_in_scope(Var * v, FuncInfo * s);

// debugging

static const bool debug = false;

BindingAnalysis::BindingAnalysis(FuncInfo * root)
  : m_root(root)
  {}

void BindingAnalysis::perform_analysis() {
  assign_scopes();
  //  find_free_mentions();  // do we want functions to contain mentions located elsewhere?
  // Maybe save this in case we want "get_may_mod" or similar
}

//! Do a bottom-up traversal of the scope tree to find the variables
//! that may be mentioned free in that scope, either explicitly
//! mentioned or mentioned in a descendant scope.
void BindingAnalysis::find_free_mentions() {
  // postorder traversal of scope tree
  FuncInfoIterator fii(m_root, PostOrder);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo * finfo = fii.Current();
    if (debug) {
      std::cout << "In function:\n";
      Rf_PrintValue(finfo->getDefn());
    }
    if (finfo != m_root) {
      // for each mention
      FuncInfo::mentions_iterator mi;
      for(mi = finfo->beginMentions(); mi != finfo->endMentions(); ++mi) {
	Var * v = *mi;
	Var::ScopeT scope_type = v->getScopeType();
	if (scope_type == Var::Var_FREE ||
	    scope_type == Var::Var_INDEFINITE)
	{
	  if (debug) {
	    std::cout << "Inserting variable ";
	    v->dump(std::cout);
	    std::cout << " into parent scope" << std::endl;
	  }
	  finfo->Parent()->insertMention(v);
	}
      }
    }
  }
}

//! Do a top-down traversal of the scope tree to find (if it exists)
//! the unique ancestor scope in which each free mention in s may be
//! bound.
void BindingAnalysis::assign_scopes() {
  // each scope
  FuncInfoIterator fii(m_root, PreOrder);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo * fi = fii.Current();
    // each free mention
    FuncInfo::mentions_iterator mi;
    for (mi = fi->beginMentions(); mi != fi->endMentions(); ++mi) {
      Var * v = *mi;
      if (v->getScopeType() != Var::Var_FREE) continue;
      FuncInfo * defining_scope = 0;
      // start at this scope's parent; iterate upward through ancestors
      FuncInfo * a;
      for(a = fi->Parent(); a != 0; a = a->Parent()) {
	if (defined_local_in_scope(v,a)) {
	  if (defining_scope == 0) {
	    defining_scope = a;
	    v->setScopeType(Var::Var_FREE_ONE_SCOPE);
	  } else {
	    // defined in more than one ancestor scope
	    v->setScopeType(Var::Var_FREE);
	    break;     // stop searching through ancestors
	  }
	}
      }
      if (defining_scope == 0) {  // nothing found at any ancestor scope
	v->setScopeType(Var::Var_GLOBAL);
      }
    }
  }
}

//! Is the given variable defined as local in the given scope?
static bool defined_local_in_scope(Var * v, FuncInfo * s) {
  FuncInfo::mentions_iterator mi;
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
