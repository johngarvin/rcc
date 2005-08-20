#include <OpenAnalysis/SSA/ManagerSSAStandard.hpp>

#include <Parser.h>
#include <TransformMatMul.h>

#include "IRInterface.h"
#include "Analyst.h"
#include "UseDefSolver.h"

using namespace std;
using namespace OA;

void opt_matmul_test(OA_ptr<CFG::CFGIRInterface> rir_ptr, SEXP exp);
void ssa_test(OA_ptr<R_IRInterface> rir_ptr, SEXP exp);
void scope_test(SEXP exp);
void uses_defs_test(OA_ptr<R_IRInterface> rir_ptr, SEXP e);
void dfa_test(OA_ptr<R_IRInterface> rir_ptr, SEXP exp);

int main(int argc, char *argv[]) {
  SEXP exp;
  init_R();
  exp = parse_R_as_function(stdin);
  OA_ptr<R_IRInterface> rir_ptr; rir_ptr = new R_IRInterface;

  //opt_matmul_test(rir_ptr, exp);
  //  ssa_test(rir_ptr, exp);
  //  scope_test(exp);
  //  uses_defs_test(rir_ptr, exp);
  dfa_test(rir_ptr, exp);
}

void opt_matmul_test(OA_ptr<CFG::CFGIRInterface> rir_ptr, SEXP exp) {
  // CFG::ManagerStandard cfg_man(rir_ptr);    

  exp = opt_matmul(exp);

  // print out program
  R_Analyst an(exp);
  OA_ptr<RScopeTree> t = an.get_scope_tree();

  // each procedure
  RScopeTree::iterator proc_iter;
  for(proc_iter = t->begin(); proc_iter != t->end(); ++proc_iter) {
    // top procedure is nil; skip it
    if (proc_iter == t->begin()) {
      assert((*proc_iter)->get_defn() == R_NilValue);
      continue;
    }
    cout << "Procedure:\n";
    PrintValue((*proc_iter)->get_defn());

    // each statement
    OA_ptr<IRRegionStmtIterator> stmt_iter_ptr;
    ProcHandle ph((irhandle_t)(*proc_iter)->get_defn());
    stmt_iter_ptr = rir_ptr->procBody(ph);
    for( ; stmt_iter_ptr->isValid(); ++*stmt_iter_ptr) {
      cout << "Statement:\n";
      PrintValue((SEXP)stmt_iter_ptr->current().hval());
    }
    // OA_ptr<CFG::CFGStandard> cfg_ptr;
    // cfg_ptr = cfg_man.performAnalysis(*proc_iter);
    // cfg_ptr->dump(cout, rir_ptr);
    // cfg_ptr->dumpdot(cout, rir_ptr);
  }
}

void ssa_test(OA_ptr<R_IRInterface> rir_ptr, SEXP exp) {
  R_Analyst an(exp);
  CFG::ManagerStandard cfg_man(rir_ptr);
  SSA::ManagerStandard ssa_man(rir_ptr);
  OA_ptr<RScopeTree> t = an.get_scope_tree();
  RScopeTree::iterator proc_iter;
  // each procedure...
  for(proc_iter = t->begin(); proc_iter != t->end(); ++proc_iter) {
    SEXP r_exp = (*proc_iter)->get_defn();
    // top procedure is nil; skip it
    if (proc_iter == t->begin()) {
      assert(r_exp == R_NilValue);
      continue;
    }
    cout << "New procedure:\n";
    PrintValue(r_exp);
    cout << "SSA info:" << endl;
    OA_ptr<CFG::CFGStandard> cfg_ptr;
    cfg_ptr = cfg_man.performAnalysis((irhandle_t)r_exp);
    cfg_ptr->dump(cout, rir_ptr);
    OA_ptr<SSA::SSAStandard> ssa_ptr;
    ssa_ptr = ssa_man.performAnalysis((irhandle_t)r_exp, cfg_ptr);
    ssa_ptr->dump(cout);
  } // procedures
}

void scope_test(SEXP e) {
  R_Analyst an(e);
  OA_ptr<RScopeTree > t = an.get_scope_tree();
  cout << "Scopes: " << t->size() << endl;
  for(RScopeTree::iterator i = t->begin(); i != t->end(); ++i) {
    PrintValue((*i)->get_defn());
    cout << endl;
  }
}

void uses_defs_test(OA_ptr<R_IRInterface> rir_ptr, SEXP e) {
  R_Analyst an(e);
  CFG::ManagerStandard cfg_man(rir_ptr);
  SSA::ManagerStandard ssa_man(rir_ptr);
  OA_ptr<RScopeTree > t = an.get_scope_tree();
  RScopeTree::iterator proc_iter;
  // each procedure...
  for(proc_iter = t->begin(); proc_iter != t->end(); ++proc_iter) {
    SEXP r_exp = (*proc_iter)->get_defn();
    if (proc_iter == t->begin()) {
      assert(r_exp == R_NilValue);
      continue;
    }
    cout << "New procedure:\n";
    PrintValue(r_exp);
    OA_ptr<CFG::Interface> cfg;
    cfg = cfg_man.performAnalysis((irhandle_t)r_exp);
    OA_ptr<CFG::Interface::NodesIterator> bb_iter;
    bb_iter = cfg->getNodesIterator();
    // each basic block (CFG node)
    for ( ; bb_iter->isValid(); ++*bb_iter) {
      cout << "CFG node #" << bb_iter->current()->getId() << ":" << endl;
      OA_ptr<CFG::Interface::NodeStatementsIterator> stmt_iter_ptr;
      stmt_iter_ptr = bb_iter->current()->getNodeStatementsIterator();
      // each statement...
      for( ; stmt_iter_ptr->isValid(); ++*stmt_iter_ptr) {
	const SEXP stmt = (SEXP)stmt_iter_ptr->current().hval();
	cout << "New statement:" << endl;
	PrintValue(stmt);
	R_ExpUDInfo info(stmt);
	OA_ptr<VarSet> vars;
	OA_ptr<VarSetIterator> it;
	
	cout << "Local defs:" << endl;
	vars = info.get_local_defs();
	for (it = vars->get_iterator(); it->isValid(); ++*it) {
	  PrintValue(it->current());
	  cout << endl;
	}

	cout << "Free defs:" << endl;
	vars = info.get_free_defs();
	for (it = vars->get_iterator(); it->isValid(); ++*it) {
	  PrintValue(it->current());
	  cout << endl;
	}

	cout << "Application uses:" << endl;
	vars = info.get_app_uses();
	for (it = vars->get_iterator(); it->isValid(); ++*it) {
	  PrintValue(it->current());
	  cout << endl;
	}

	cout << "Non-application uses:" << endl;
	vars = info.get_non_app_uses();
	for (it = vars->get_iterator(); it->isValid(); ++*it) {
	  PrintValue(it->current());
	  cout << endl;
	}
      } // statements
    } // basic blocks (CFG nodes)
  } // procedures
}

void dfa_test(OA_ptr<R_IRInterface> rir_ptr, SEXP e) {
  CFG::ManagerStandard cfg_man(rir_ptr, true);         // statement-level CFG
  OA_ptr<CFG::Interface> cfg_ptr;
  OA_ptr<RAnnot::AnnotationSet> aset;
  R_Analyst an(e);
  OA_ptr<RScopeTree> t = an.get_scope_tree();
  for(RScopeTree::iterator i = t->begin(); i != t->end(); ++i) {
    cout << "New scope tree procedure" << endl;
    SEXP fundef = (*i)->get_defn();
    if (i == t->begin()) { // top of scope tree is defined as nil
      assert(fundef == R_NilValue);
      continue;
    }
    ProcHandle ph((irhandle_t)fundef);
    cfg_ptr = cfg_man.performAnalysis(ph);
    cfg_ptr->dump(cout, rir_ptr);
    R_UseDefSolver uds(rir_ptr);
    aset = uds.perform_analysis(ph, cfg_ptr);
    uds.dump_node_maps();
  }
}
