#include <OpenAnalysis/SSA/ManagerSSAStandard.hpp>

#include <support/Parser.h>
#include <matmul/TransformMatMul.h>

#include <analysis/IRInterface.h>
#include <analysis/Analyst.h>
#include <analysis/UseDefSolver.h>

using namespace std;
using namespace OA;
using namespace RAnnot;

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
  FuncInfo *scopeTree = an.get_scope_tree();

  // each procedure
  FuncInfoIterator fii(scopeTree);
  FuncInfo *root = fii.Current();
  for(FuncInfo *fi; fi = fii.Current(); fii++) {
    fi->dump(cout);

#if 0
    // each statement
    OA_ptr<IRRegionStmtIterator> stmt_iter_ptr;
    ProcHandle ph((irhandle_t)fi->get_defn());
    stmt_iter_ptr = rir_ptr->procBody(ph);
    for( ; stmt_iter_ptr->isValid(); ++*stmt_iter_ptr) {
      cout << "Statement:\n";
      Rf_PrintValue((SEXP)stmt_iter_ptr->current().hval());
    }
    // OA_ptr<CFG::CFGStandard> cfg_ptr;
    // cfg_ptr = cfg_man.performAnalysis(*proc_iter);
    // cfg_ptr->dump(cout, rir_ptr);
    // cfg_ptr->dumpdot(cout, rir_ptr);
#endif
  }
}

void ssa_test(OA_ptr<R_IRInterface> rir_ptr, SEXP exp) {
  R_Analyst an(exp);
  CFG::ManagerStandard cfg_man(rir_ptr);
  SSA::ManagerStandard ssa_man(rir_ptr);
  FuncInfo *scopeTree = an.get_scope_tree();
  FuncInfoIterator fii(scopeTree);
  // each procedure...
  for(FuncInfo *fi; fi = fii.Current(); fii++) {
    SEXP r_exp = fi->get_defn();

    // skip nil procedures
    if (r_exp == R_NilValue) continue;

    fi->dump(cout);

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
  FuncInfo *scopeTree = an.get_scope_tree();
  cout << "Scopes: " << endl;
  FuncInfoIterator fii(scopeTree);
  for(FuncInfo *fi; fi = fii.Current(); fii++) {
    fi->dump(cout);
  }
}

void uses_defs_test(OA_ptr<R_IRInterface> rir_ptr, SEXP e) {
  R_Analyst an(e);
  CFG::ManagerStandard cfg_man(rir_ptr);
  SSA::ManagerStandard ssa_man(rir_ptr);
  FuncInfo *scopeTree = an.get_scope_tree();
  FuncInfoIterator fii(scopeTree);
  for(FuncInfo *fi; fi = fii.Current(); fii++) {
    SEXP r_exp = fi->get_defn();
    if (r_exp == R_NilValue) continue;

    cout << "New procedure:\n";
    fi->dump(cout);

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
	Rf_PrintValue(stmt);
	R_ExpUDLocInfo info(stmt);
	OA_ptr<R_VarRefSet> vars;
	OA_ptr<R_VarRefSetIterator> it;
	
	cout << "Local defs:" << endl;
	vars = info.get_local_defs();
	for (it = vars->get_iterator(); it->isValid(); ++*it) {
	  cout << it->current()->toString() << endl;
	}

	cout << "Free defs:" << endl;
	vars = info.get_free_defs();
	for (it = vars->get_iterator(); it->isValid(); ++*it) {
	  cout << it->current()->toString() << endl;
	}

	cout << "Application uses:" << endl;
	vars = info.get_app_uses();
	for (it = vars->get_iterator(); it->isValid(); ++*it) {
	  cout << it->current()->toString() << endl;
	}

	cout << "Non-application uses:" << endl;
	vars = info.get_non_app_uses();
	for (it = vars->get_iterator(); it->isValid(); ++*it) {
	  cout << it->current()->toString() << endl;
	}
      } // statements
    } // basic blocks (CFG nodes)
  } // procedures
}

void dfa_test(OA_ptr<R_IRInterface> rir_ptr, SEXP e) {
  //  CFG::ManagerStandard cfg_man(rir_ptr, true);         // statement-level CFG
  CFG::ManagerStandard cfg_man(rir_ptr, false);         // temp. turn off statement-level CFG
  OA_ptr<CFG::Interface> cfg_ptr;
  OA_ptr<RAnnot::AnnotationSet> aset;
  R_Analyst an(e);
  FuncInfo *scopeTree = an.get_scope_tree();
  FuncInfoIterator fii(scopeTree);
  for(FuncInfo *fi; fi = fii.Current(); fii++) {
    cout << "New scope tree procedure" << endl;
    SEXP fundef = fi->get_defn();
    if (fundef == R_NilValue) continue;

    ProcHandle ph((irhandle_t)fundef);
    cfg_ptr = cfg_man.performAnalysis(ph);
    cfg_ptr->dump(cout, rir_ptr);
    R_UseDefSolver uds(rir_ptr);
    aset = uds.perform_analysis(ph, cfg_ptr);
    uds.dump_node_maps();
  }
}
