// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
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

// File: Analyst.cc
//
// Singleton manager for annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <algorithm>

#include <ParseInfo.h>

#include <support/RccError.h>

#include <analysis/AnalysisException.h>
#include <analysis/Utils.h>
#include <analysis/SimpleIterators.h>
#include <analysis/AnalysisResults.h>
#include <analysis/HandleInterface.h>
#include <analysis/FuncInfo.h>
#include <analysis/LocalFunctionAnalysis.h>

#include "Analyst.h"

using namespace RAnnot;
using namespace RProp;
using namespace HandleInterface;

// ----- implement Singleton pattern -----

/// the only instance
R_Analyst * R_Analyst::m_instance = 0;

/// static instantiation
R_Analyst * R_Analyst::get_instance(SEXP _program) {
  if (m_instance == 0) {
    m_instance = new R_Analyst(_program);
  }
  return m_instance;
}

/// just get the existing instance; error if not yet instantiated
R_Analyst * R_Analyst::get_instance() {
  if (m_instance == 0) {
    rcc_error("R_Analyst is not yet instantiated");
  }
  return m_instance;
}

// ----- constructor -----

/// construct an R_Analyst by providing an SEXP representing the whole program
R_Analyst::R_Analyst(SEXP _program)
  : m_program(_program),
    m_interface(),
    m_scope_tree_root(0),
    m_library_scope(0),
    m_global_scope(0)
  {}

// ----- analysis -----

bool R_Analyst::perform_analysis() {
  try {
    // initialize what's not initialized by the constructor
    m_interface = new R_IRInterface();
    m_scope_tree_root = getProperty(FuncInfo, CAR(assign_rhs_c(m_program)));
    m_global_scope = m_scope_tree_root->get_scope();
    m_library_scope = new InternalLexicalScope();

    if (ParseInfo::allow_oo()           ||
	ParseInfo::allow_envir_manip())
    {
      throw AnalysisException();
    }
    build_cfgs();
    build_local_function_info();
    return true;
  }
  catch (AnalysisException ae) {
    // One phase of analysis rejected a program. Get rid of the
    // information in preparation to compile trivially.
    rcc_warn("analysis encountered difficulties; compiling trivially");
    clearProperties();
    return false;
  }
}

FuncInfo * R_Analyst::get_scope_tree_root() {
  return m_scope_tree_root;
}

LexicalScope * R_Analyst::get_library_scope() {
  return m_library_scope;
}

LexicalScope * R_Analyst::get_global_scope() {
  return m_global_scope;
}

/// Dump the CFG of the given function definition. Located here in
/// R_Analyst because cfg->dump needs the IRInterface as an argument.
void R_Analyst::dump_cfg(std::ostream &os, SEXP proc) {
  if (proc != R_NilValue) {
    FuncInfo *fi = getProperty(FuncInfo, proc);
    OA::OA_ptr<OA::CFG::Interface> cfg; cfg = fi->get_cfg();
    cfg->dump(os, m_interface);
  }
}

void R_Analyst::dump_all_cfgs(std::ostream &os) {
  FuncInfoIterator fii(m_scope_tree_root);
  for( ; fii.IsValid(); ++fii) {
    FuncInfo *finfo = fii.Current();
    OA::OA_ptr<OA::CFG::Interface> cfg; cfg = finfo->get_cfg();
    cfg->dump(os, m_interface);
  }
}

/// Populate m_cfgs with the CFG for each procedure
void R_Analyst::build_cfgs() {
  OA::CFG::ManagerStandard cfg_man(m_interface, true); // build statement-level CFGs

  // preorder traversal of scope tree
  FuncInfoIterator fii(get_scope_tree_root());
  for( ; fii.IsValid(); ++fii) {
    OA::ProcHandle ph = make_proc_h(fii.Current()->get_defn());
    OA::OA_ptr<OA::CFG::CFGStandard> cfg_ptr; cfg_ptr = cfg_man.performAnalysis(ph);
    fii.Current()->set_cfg(cfg_ptr);
  }
}

#if 0
/// Collect basic local info on variables: use/def, "<-"/"<<-", etc.
void R_Analyst::build_local_variable_info() {
  // each function
  FuncInfoIterator fii(m_scope_tree_root);
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    OA::OA_ptr<OA::CFG::Interface> cfg; cfg = fi->getCFG();

    // each node in function's CFG
    OA::OA_ptr<OA::CFG::Interface::NodesIterator> ni = cfg->getNodesIterator();
    for ( ; ni->isValid(); ++*ni) {

      // each statement in node
      OA::OA_ptr<OA::CFG::Interface::NodeStatementsIterator> si;
      si = ni->current()->getNodeStatementsIterator();
      for( ; si->isValid(); ++*si) {
	OA::StmtHandle stmt = si->current();
	LocalVariableAnalysis lva(make_sexp(stmt));
	lva.perform_analysis();
      }
    }
  }
}
#endif

/// Discovers local information on procedures: arguments, names
/// mentioned, etc.
void R_Analyst::build_local_function_info() {
  FuncInfoIterator fii(m_scope_tree_root);
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    LocalFunctionAnalysis lfa(fi->get_defn());
    lfa.perform_analysis();
  }
}

#if 0
/// For each procedure, use control flow to discover locality for each
/// name (whether local or free)
void R_Analyst::build_locality_info() {
  FuncInfoIterator fii(m_scope_tree_root);
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    Locality::LocalityDFSolver uds(m_interface);
    OA::ProcHandle ph = make_proc_h(fi->get_defn());
    uds.perform_analysis(ph, fi->getCFG());
  }
}
#endif

/// Perform binding analysis to resolve names to a single scope if
/// possible
void R_Analyst::build_bindings() {
  //  BindingAnalysis ba(m_scope_tree_root);
  //  ba.perform_analysis();
}

#if 0
/// For each procedure, discover which other procedures are called
void R_Analyst::build_call_graph() {
  CallGraphBuilder cgb(m_scope_tree_root);
  cgb.perform_analysis();
}
#endif
