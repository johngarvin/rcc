// -*- Mode: C++ -*-

// #include <support/StringUtils.h>

#include "CallGraphBuilder.h"

using namespace RAnnot;

CallGraphBuilder::CallGraphBuilder(FuncInfo* root)
  : m_root(root)
  {}

void CallGraphBuilder::perform_analysis()
{
  // each scope
  for(FuncInfoIterator fii(m_root, PreOrder); fii.IsValid(); ++fii) {
    FuncInfo* fi = fii.Current();
    // for each mention
    for(FuncInfo::mention_iterator mi = fi->beginMentions(); mi != fi->endMentions(); ++mi) {
      UseVar* u; u = dynamic_cast<UseVar*>(*mi);
      if (u != 0 && u->getPositionType() == UseVar::UseVar_FUNCTION) {
	FuncInfo* scope = u->getBoundScope();
	if (scope == 0) {
	  // FIXME: what should be done in this case?
	} else {
	  SymbolTable* st = scope->getSymbolTable();
	  SymbolTable::iterator info = st->find(u->getName());
	  if (info != st->end() && info->second->isSingleton()) {
	    FuncInfo* callee = info->second->getSingletonDef();
	    if (callee != 0) {
	      fi->insertCallOut(callee);
	      callee->insertCallIn(fi);
	    }
	  }
	}
      }
    }
  }
}
