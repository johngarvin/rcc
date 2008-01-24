// -*- Mode: C++ -*-
//
// Copyright (c) 2007 Rice University
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

// File: OACallGraphAnnotationMap.cc
//
// Represents the OpenAnalysis call graph of the program.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/CallGraph/ManagerCallGraph.hpp>
#include <OpenAnalysis/Alias/InterAliasMap.hpp>
#include <OpenAnalysis/Alias/ManagerInterAliasMapBasic.hpp>
#include <OpenAnalysis/DataFlow/CallGraphDFSolver.hpp>
#include <OpenAnalysis/DataFlow/DGraphSolverDFP.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/Utils/OutputBuilderDOT.hpp>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/OACallGraphAnnotation.h>

#include <support/RccError.h>

#include "OACallGraphAnnotationMap.h"

using namespace OA;

namespace RAnnot {

// ----- type definitions for readability -----
  
typedef OACallGraphAnnotationMap::MyKeyT MyKeyT;
typedef OACallGraphAnnotationMap::MyMappedT MyMappedT;
typedef OACallGraphAnnotationMap::iterator iterator;
typedef OACallGraphAnnotationMap::const_iterator const_iterator;

// ----- constructor, destructor -----

OACallGraphAnnotationMap::OACallGraphAnnotationMap() {}

OACallGraphAnnotationMap::~OACallGraphAnnotationMap() {
  delete_map_values();
}  

// ----- singleton pattern -----

OACallGraphAnnotationMap * OACallGraphAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}
  
PropertyHndlT OACallGraphAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}
  
void OACallGraphAnnotationMap::create() {
  m_instance = new OACallGraphAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}
  
OACallGraphAnnotationMap * OACallGraphAnnotationMap::m_instance = 0;
PropertyHndlT OACallGraphAnnotationMap::m_handle = "OACallGraph";

// ----- demand-driven analysis -----

/// given a call site, return the list of fundef nodes reachable; compute if necessary
// overrides DefaultAnnotationMap::get
MyMappedT OACallGraphAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
  }
  
  if (is_valid(k)) {
    return get_map()[k];
  } else {
    OA_ptr<ProcHandleIterator> iter = m_call_graph->getCalleeProcIter(HandleInterface::make_call_h(k));
    // only populate map if there's at least one callee (iterator is nonempty)
    if (iter->isValid()) {
      get_map()[k] = new OACallGraphAnnotation(iter);
      return get_map()[k];
    } else {
      return 0;
    }
  }
}

void OACallGraphAnnotationMap::compute() {
  OA_ptr<R_IRInterface> interface; interface = R_Analyst::get_instance()->get_interface();
  // first build call graph. The call graph manager needs
  // (1) a procedure iterator and (2) alias information
  CallGraph::ManagerCallGraphStandard man(interface);

  // (1) procedure iterator
  OA_ptr<ProcHandleIterator> proc_iter;
  proc_iter = new R_ProcHandleIterator(R_Analyst::get_instance()->get_scope_tree_root());

  // (2) alias information
  OA_ptr<Alias::ManagerInterAliasMapBasic> alias_man;
  alias_man = new Alias::ManagerInterAliasMapBasic(interface);
  m_alias = alias_man->performAnalysis(proc_iter);

  // build call graph
  m_call_graph = man.performAnalysis(proc_iter, m_alias);
  
  // now perform call graph data flow analysis (specifically, interprocedural side effect).
  // We need (3) param bindings and (4) intraprocedural side effect information
  SideEffect::ManagerInterSideEffectStandard solver(interface);

  // (3) param bindings
  DataFlow::ManagerParamBindings pb_man(interface);
  OA_ptr<DataFlow::ParamBindings> param_bindings = pb_man.performAnalysis(m_call_graph);

  // (4) intra side effect information
  OA_ptr<SideEffect::ManagerSideEffectStandard> intra_man;
  intra_man = new SideEffect::ManagerSideEffectStandard(interface);

  // compute side effect information
  m_side_effect = solver.performAnalysis(m_call_graph, param_bindings, m_alias, intra_man, DataFlow::ITERATIVE);
}

// ----- debugging -----

void OACallGraphAnnotationMap::dump(std::ostream & os) {
  if (!is_computed()) {
    compute();
  }
  OA_ptr<R_IRInterface> interface; interface = R_Analyst::get_instance()->get_interface();
  m_call_graph->output(*interface);
  m_side_effect->dump(cout, interface);
}

void OACallGraphAnnotationMap::dumpdot(std::ostream & os) {
  if (!is_computed()) {
    compute();
  }
  //   output graph in DOT form
  OA_ptr<OutputBuilder> dot_builder; dot_builder = new OutputBuilderDOT;
  m_call_graph->configOutput(dot_builder);
  m_call_graph->output(*R_Analyst::get_instance()->get_interface());
}

// ----- access to OA call graph -----

OA_ptr<CallGraph::CallGraphInterface> OACallGraphAnnotationMap::get_OA_call_graph() {
  return m_call_graph;
}

OA_ptr<Alias::InterAliasInterface> OACallGraphAnnotationMap::get_OA_alias() {
  return m_alias;
}


} // namespace RAnnot
