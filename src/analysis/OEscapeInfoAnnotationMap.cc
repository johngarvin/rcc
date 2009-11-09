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
#include <OpenAnalysis/SSA/ManagerSSAStandard.hpp>
#include <OpenAnalysis/SSA/SSAStandard.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/EscapedDFSolver.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/NameBoolDFSet.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/OEscapeDFSolver.h>
#include <analysis/OEscapeInfo.h>
#include <analysis/PropertyHndl.h>
#include <analysis/ReturnedDFSolver.h>
#include <analysis/SymbolTable.h>
#include <analysis/Var.h>
#include <analysis/VarInfo.h>
#include <analysis/VarBinding.h>

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
  OA_ptr<CFG::Node> node;
  StmtHandle stmt;
  OA_ptr<R_IRInterface> interface; interface = R_Analyst::get_instance()->get_interface();
  SSA::ManagerStandard ssa_man(interface);
  OA_ptr<SSA::SSAStandard> ssa;
  OA_ptr<NameBoolDFSet::NameBoolDFSetIterator> iter;

  // get intraprocedural data for each procedure
  FOR_EACH_PROC(fi) {
    ProcHandle proc = HandleInterface::make_proc_h(fi->get_sexp());
    ssa = ssa_man.performAnalysis(proc, fi->get_cfg());
    ReturnedDFSolver ret_solver(interface);
    OA::OA_ptr<NameBoolDFSet> returned; returned = ret_solver.perform_analysis(proc, fi->get_cfg());
    EscapedDFSolver esc_solver(interface);
    OA::OA_ptr<NameBoolDFSet> escaped; escaped = esc_solver.perform_analysis(proc, fi->get_cfg());
    OEscapeDFSolver oe_solver(interface);
    OA::OA_ptr<NameBoolDFSet> oe; oe = oe_solver.perform_analysis(proc, fi->get_cfg());
    for (iter = oe->getIterator(); iter->isValid(); ++*iter) {
      bool ret = returned->lookup(iter->current()->getName());
      bool esc = escaped->lookup(iter->current()->getName());
      bool nfresh = iter->current()->getValue();
      if (ret || esc || nfresh) {
	// may escape
	get_map()[iter->current()->getName()->get_name()] = new OEscapeInfo(true);
      } else {
	// does not escape
	get_map()[iter->current()->getName()->get_name()] = new OEscapeInfo(false);
      }
    }
  }

  // use call graph to get interprocedural data
  OA_ptr<OA::CallGraph::CallGraphInterface> call_graph;
  call_graph = RAnnot::OACallGraphAnnotationMap::get_instance()->get_OA_call_graph();
  
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
