// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 

// File: OEscapeInfoAnnotationMap.cc
//
// Maps expression SEXPs to information from object escape analysis.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <map>

#include <OpenAnalysis/CFG/CFG.hpp>
#include <OpenAnalysis/DataFlow/CallGraphDFSolver.hpp>
#include <OpenAnalysis/SSA/ManagerSSAStandard.hpp>
#include <OpenAnalysis/SSA/SSAStandard.hpp>

#include <support/Debug.h>
#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/EscapedCGSolver.h>
#include <analysis/ExpressionDFSet.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/HellProcedure.h>
#include <analysis/MFreshCGSolver.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/OEscapeDFSolver.h>
#include <analysis/OEscapeInfo.h>
#include <analysis/PropertyHndl.h>
#include <analysis/ReturnedCGSolver.h>
#include <analysis/ReturnedDFSolver.h>
#include <analysis/SymbolTable.h>
#include <analysis/Var.h>
#include <analysis/VarInfo.h>
#include <analysis/VarBinding.h>
#include <analysis/VFreshDFSolver.h>

#include "OEscapeInfoAnnotationMap.h"

using namespace OA;

static bool debug;

namespace RAnnot {

// ----- declarations for static functions -----

static std::vector<SEXP> implicit_return_exps(SEXP fundef);

// ----- type definitions for readability -----

typedef OEscapeInfoAnnotationMap::MyKeyT MyKeyT;
typedef OEscapeInfoAnnotationMap::MyMappedT MyMappedT;

// ----- constructor/destructor ----- 

OEscapeInfoAnnotationMap::OEscapeInfoAnnotationMap()
{
  RCC_DEBUG("RCC_OEscapeInfoAnnotationMap", debug);
}

OEscapeInfoAnnotationMap::~OEscapeInfoAnnotationMap() {
  // owns OEscapeInfo annotations, so delete them in deconstructor
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = get_map().begin(); iter != get_map().end(); ++iter) {
    delete(iter->second);
  }
}


// ----- computation -----

void OEscapeInfoAnnotationMap::compute() {
  FuncInfo * fi;
  Var * var;
  StmtHandle stmt;
  ProcHandle proc;
  OA_ptr<R_IRInterface> interface; interface = R_Analyst::get_instance()->get_interface();
  SSA::ManagerStandard ssa_man(interface);
  OA_ptr<CFG::CFGInterface> cfg;
  //  OA_ptr<SSA::SSAStandard> ssa;
  OA_ptr<CallGraph::NodeInterface> cg_node;
  OA_ptr<ExpressionDFSet> returned; 
  OA_ptr<ExpressionDFSet> escaped;
  OA_ptr<ExpressionDFSet> nmfresh;
  OA_ptr<ExpressionDFSet> nvfresh;
  //  OA_ptr<ExpressionDFSet> oe;
  OA_ptr<ExpressionDFSet::ExpressionDFSetIterator> iter;
  OA_ptr<CallGraph::NodesIteratorInterface> cg_iter;

  // use call graph to get interprocedural data
  OA_ptr<OA::CallGraph::CallGraphInterface> call_graph;
  call_graph = RAnnot::OACallGraphAnnotationMap::get_instance()->get_OA_call_graph();
  if (debug) {
    std::cout << "Call graph annotation map:\n";
    RAnnot::OACallGraphAnnotationMap::get_instance()->dump(std::cout);
  }
  ReturnedCGSolver * ret_problem = new ReturnedCGSolver();
  ret_problem->perform_analysis(call_graph, DataFlow::ITERATIVE);
  EscapedCGSolver * esc_problem = new EscapedCGSolver();
  esc_problem->perform_analysis(call_graph, DataFlow::ITERATIVE);
  MFreshCGSolver * mfresh_problem = new MFreshCGSolver();
  mfresh_problem->perform_analysis(call_graph, DataFlow::ITERATIVE);

  // get intraprocedural data for each procedure in call graph
  cg_iter = call_graph->getCallGraphNodesIterator();
  for(cg_iter->reset(); cg_iter->isValid(); ++*cg_iter) {
    cg_node = cg_iter->currentCallGraphNode();
    proc = cg_node->getProc();
    if (HandleInterface::make_sexp(proc) == 0) {
      rcc_warn("OEscapeInfoAnnotationMap: no data for unknown procedure");
      continue;
    }
    fi = getProperty(FuncInfo, HandleInterface::make_sexp(proc));
    cfg = fi->get_cfg();
    //    ssa = ssa_man.performAnalysis(proc, cfg);
    
    if (fi == R_Analyst::get_instance()->get_scope_tree_root()) {
      // The scope of the whole program. Obviously we don't care about
      // escapes/returns from the global scope.
      returned = new ExpressionDFSet();
      escaped = new ExpressionDFSet();
      nmfresh = new ExpressionDFSet();
      nvfresh = new ExpressionDFSet();
    } else {
      returned = ret_problem->getOutSet(cg_node).convert<ExpressionDFSet>();
      escaped = esc_problem->getOutSet(cg_node).convert<ExpressionDFSet>();
      nmfresh = mfresh_problem->getOutSet(cg_node).convert<ExpressionDFSet>();
      VFreshDFSolver * nvfresh_problem = new VFreshDFSolver(interface);
      nvfresh = nvfresh_problem->perform_analysis(proc, cfg);
    }
    if (debug) {
      std::cout << "OEscape info:" << std::endl;
      std::cout << var_name(CAR(fi->get_first_name_c())) << std::endl;
      std::cout << "Returned:" << std::endl;
      returned->dump(std::cout);
      std::cout << "Escaped:" << std::endl;
      escaped->dump(std::cout);
      std::cout << "NMFresh:" << std::endl;
      nmfresh->dump(std::cout);
      std::cout << "NVFresh:" << std::endl;
      nvfresh->dump(std::cout);
    }
    //    OEscapeDFSolver * oe_solver = new OEscapeDFSolver(interface);
    //    oe = oe_solver->perform_analysis(proc, cfg);
    //    for (iter = oe->getIterator(); iter->isValid(); ++*iter) {
    OA_ptr<CFG::NodeInterface> node;
    StmtHandle stmt;
    PROC_FOR_EACH_NODE(fi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	ExpressionInfo * ei; ei = getProperty(ExpressionInfo, HandleInterface::make_sexp(stmt));
	SEXP cs;
	EXPRESSION_FOR_EACH_CALL_SITE(ei, cs) {
	  bool ret = returned->lookup(cs);
	  bool esc = escaped->lookup(cs);
	  bool nf = nvfresh->lookup(cs);
	  if (debug) {
	    std::cout << "{Call site: ";
	    printf("%x ", cs);
	    Rf_PrintValue(CAR(cs));
	    std::cout << "(ret = " << ret << ", esc = " << esc << ", nf = " << nf << ")}\n";
	  }
	  if (ret || esc || nf) {
	    // may escape
	    get_map()[cs] = new OEscapeInfo(true);
	  } else {
	    // does not escape
	    get_map()[cs] = new OEscapeInfo(false);
	  }
	}
      }
    }
  }
}

#if 0
  /*
	  } else if (is_library(call_lhs(e))) {
	    CEscapeInfo * ei;
	    PRIMPOINTS(library_value(call_lhs(e)))
	      PRIMESCAPE(library_value(call_lhs(e)))
  */
#endif

#if 0 
void compute_returned() {
  std::map<SEXP, bool> returned;
  FuncInfo * fi;
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;

  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_NODE(fi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	
      }
    }
  }
}
#endif


// ----- singleton pattern -----

OEscapeInfoAnnotationMap * OEscapeInfoAnnotationMap::get_instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT OEscapeInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void OEscapeInfoAnnotationMap::create() {
  s_instance = new OEscapeInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

OEscapeInfoAnnotationMap * OEscapeInfoAnnotationMap::s_instance = 0;
PropertyHndlT OEscapeInfoAnnotationMap::s_handle = "OEscapeInfo";


} // end namespace RAnnot
