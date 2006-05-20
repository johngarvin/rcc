#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/Interface.hpp>

#include <analysis/Utils.h>
#include <analysis/AnalysisResults.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/Var.h>
#include <analysis/DefVar.h>

#include "LocalFunctionAnalysis.h"

using namespace OA;
using namespace RAnnot;

LocalFunctionAnalysis::LocalFunctionAnalysis(const SEXP fundef)
  : m_fundef(fundef)
{
  assert(is_fundef(fundef));
}

/// Discovers local information about the given function
/// definition. Adds annotations to the global AnalysisResults.
void LocalFunctionAnalysis::perform_analysis() {
  analyze_args();
  collect_mentions_and_call_sites();
}

void LocalFunctionAnalysis::analyze_args() {
  SEXP args = CAR(fundef_args_c(m_fundef));
  const SEXP ddd = Rf_install("...");
  bool has_var_args = false;
  int n_args = 0;
  for(SEXP e = args; e != R_NilValue; e = CDR(e)) {
    ++n_args;
    DefVar * annot = new DefVar();
    annot->setMention_c(e);
    annot->setSourceType(DefVar::DefVar_FORMAL);
    annot->setMayMustType(Var::Var_MUST);
    annot->setScopeType(Var::Var_LOCAL);
    annot->setRhs_c(0);
    putProperty(Var, e, annot, true);
    if (TAG(e) == ddd) {
      has_var_args = true;
    }
  }
  FuncInfo * f_annot = getProperty(FuncInfo, m_fundef);
  assert(f_annot != 0);
  f_annot->setNumArgs(n_args);
  f_annot->setHasVarArgs(has_var_args);
}

/// Find each mention (use or def) and call site in the function
void LocalFunctionAnalysis::collect_mentions_and_call_sites() {
  FuncInfo * f_annot = getProperty(FuncInfo, m_fundef);
  OA_ptr<CFG::Interface> cfg; cfg = f_annot->getCFG();
  assert(!cfg.ptrEqual(0));

  // for each node
  OA_ptr<CFG::Interface::NodesIterator> ni; ni = cfg->getNodesIterator();
  for( ; ni->isValid(); ++*ni) {
    // for each statement
    OA_ptr<CFG::Interface::NodeStatementsIterator> si;
    si = ni->current()->getNodeStatementsIterator();
    for( ; si->isValid(); ++*si) {
      // for each mention
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, HandleInterface::make_sexp(si->current()));
      assert(stmt_annot != 0);
      ExpressionInfo::const_var_iterator mi;
      for(mi = stmt_annot->begin_vars(); mi != stmt_annot->end_vars(); ++mi) {
	Var * v = getProperty(Var, (*mi)->getMention_c());
	// FIXME: should make sure we always get the data-flow-solved
	// version of the Var. Shouldn't have to loop through
	// getProperty!
	f_annot->insertMention(v);
      }
      ExpressionInfo::const_call_site_iterator cs;
      for(cs = stmt_annot->begin_call_sites(); cs != stmt_annot->end_call_sites(); ++cs) {
	f_annot->insertCallSite(*cs);
      }
    }
  }
}
