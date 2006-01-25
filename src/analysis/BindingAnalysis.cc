#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>
#include <support/RccError.h>

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
  fill_in_symbol_tables();

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
      FuncInfo::mention_iterator mi;
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
//! the unique ancestor scope in which each free mention may be bound.
//! 
//! On entry, the scope type of each Var (accessed by getScopeType) is
//! either LOCAL, FREE, or INDEFINITE, reflecting only control flow
//! information. On exit, these scope types are specialized to be one
//! of the following:
//! 
//! LOCAL: local in a non-global scope
//! INDEFINITE: may be bound in the local scope and at least one ancestor scope
//! FREE_ONE_SCOPE: free with respect to the definition, bound in a unique non-global scope
//! FREE: free variable, no unique binding
//! GLOBAL: bound in the global scope
//! 
//! Sets the bound scope as well as the scope type of each Var.
void BindingAnalysis::assign_scopes() {
  // each scope
  FuncInfoIterator fii(m_root, PreOrder);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo * fi = fii.Current();
    // each free mention
    FuncInfo::mention_iterator mi;
    for (mi = fi->beginMentions(); mi != fi->endMentions(); ++mi) {
      Var * v = *mi;
      FuncInfo * scope;
      switch(v->getScopeType()) {
      case Var::Var_INDEFINITE:
	v->setBoundScope(0);
	continue;
      case Var::Var_LOCAL:
	v->setBoundScope(fi);
	// if name is not already in fi, then create a new VarInfo...
	continue;
      case Var::Var_FREE:
	scope = 0;
	// remember if some defining scope has already been found
	// start at this scope's parent; iterate upward through ancestors
	for(FuncInfo* a = fi->Parent(); a != 0; a = a->Parent()) {
	  if (defined_local_in_scope(v,a)) {
	    if (scope == 0) {
	      scope = a;
	      v->setScopeType(Var::Var_FREE_ONE_SCOPE);
	      v->setBoundScope(a);
	    } else {
	      // defined in more than one ancestor scope
	      v->setScopeType(Var::Var_FREE);
	      v->setBoundScope(0);
	      break;     // stop searching through ancestors
	    }
	  }
	}
	// if no definition found at any ancestor scope, binding is global
	if (scope == 0) {
	  v->setScopeType(Var::Var_GLOBAL);
	  v->setBoundScope(m_root);
	}
	break;
      default:
	rcc_error("Unknown scope type encountered");
	break;
      }
    }
  }
}


//! Fill in the symbol table for each function using the binding scope
//! of each def. Also computes the singleton status of names bound to
//! functions.
void BindingAnalysis::fill_in_symbol_tables() {
  // each scope
  FuncInfoIterator fii(m_root, PreOrder);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo * fi = fii.Current();
    // each mention (defs, not uses)
    for (FuncInfo::mention_iterator mi = fi->beginMentions(); mi != fi->endMentions(); ++mi) {
      DefVar* def; def = dynamic_cast<DefVar*>(*mi);
      if (def == 0) continue;
      FuncInfo * ds = def->getBoundScope();
      if (ds == 0) {
	// FIXME: what should be done in this case?
      } else {
	SymbolTable* st = ds->getSymbolTable();
	SymbolTable::iterator info = st->find(def->getName());
	if (info == st->end()) { // not yet in symbol table
	  VarInfo * vi = new VarInfo();
	  vi->setSymbolTable(st);
	  SEXP rhs = CAR(def->getRhs());
	  if (is_fundef(rhs)) {
	    vi->setSingleton(true);
	    vi->setSingletonDef(getProperty(FuncInfo, rhs));
	  } else {
	    vi->setSingleton(false);
	    vi->setSingletonDef(0);
	  }
	  st->insert(std::pair<Name,VarInfo*>(def->getName(),vi));
	} else {
	  info->second->setSingleton(false);
	}
      }
    }
  }
}

//! Is the given variable defined as local in the given scope?
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
