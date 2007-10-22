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

OACallGraphAnnotationMap::OACallGraphAnnotationMap() : m_computed(false)
{
}

OACallGraphAnnotationMap::~OACallGraphAnnotationMap()
{
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

// TODO: remove this when refactoring is done
MyMappedT & OACallGraphAnnotationMap::operator[](const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  return m_map[k];
}

/// given a call site, return the list of fundef nodes reachable; compute if necessary
MyMappedT OACallGraphAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  std::map<MyKeyT, MyMappedT>::const_iterator it = m_map.find(k);
  if (it != m_map.end()) {
    return it->second;
  } else {
    OA_ptr<ProcHandleIterator> iter = m_call_graph->getCalleeProcIter(HandleInterface::make_call_h(k));
    // only populate map if iterator is nonempty
    if (iter->isValid()) {
      m_map[k] = new OACallGraphAnnotation(iter);
      return m_map[k];
    } else {
      return 0;
    }
  }
}


bool OACallGraphAnnotationMap::is_computed() {
  return m_computed;
}

void OACallGraphAnnotationMap::compute() {
  OA_ptr<R_IRInterface> interface; interface = R_Analyst::get_instance()->get_interface();
  // first build call graph
  OA::CallGraph::ManagerCallGraphStandard man(interface);
  OA::OA_ptr<OA::ProcHandleIterator> proc_iter;
  proc_iter = new R_ProcHandleIterator(R_Analyst::get_instance()->get_scope_tree_root());
  OA::OA_ptr<OA::Alias::ManagerInterAliasMapBasic> alias_man;
  alias_man = new OA::Alias::ManagerInterAliasMapBasic(interface);
  OA::OA_ptr<OA::Alias::InterAliasInterface> alias; alias = alias_man->performAnalysis(proc_iter);
  m_call_graph = man.performAnalysis(proc_iter, alias);
  
  // now perform call graph data flow analysis
  OA::SideEffect::ManagerInterSideEffectStandard solver(interface);
  OA::DataFlow::ManagerParamBindings pb_man(interface);
  OA::OA_ptr<OA::DataFlow::ParamBindings> param_bindings = pb_man.performAnalysis(m_call_graph);
  OA::OA_ptr<OA::SideEffect::ManagerSideEffectStandard> intra_man;
  intra_man = new OA::SideEffect::ManagerSideEffectStandard(interface);
  m_side_effect = solver.performAnalysis(m_call_graph, param_bindings, alias, intra_man, OA::DataFlow::ITERATIVE);
}

//  ----- iterators ----- 

// TODO: fill in
iterator OACallGraphAnnotationMap::begin() { rcc_error("not yet implemented"); }
iterator OACallGraphAnnotationMap::end() { rcc_error("not yet implemented"); }
const_iterator OACallGraphAnnotationMap::begin() const { rcc_error("not yet implemented"); }
const_iterator OACallGraphAnnotationMap::end() const { rcc_error("not yet implemented"); }

// ----- debugging -----
void OACallGraphAnnotationMap::dump(std::ostream & os) {
  OA_ptr<R_IRInterface> interface; interface = R_Analyst::get_instance()->get_interface();
  m_call_graph->output(*interface);
  m_side_effect->dump(cout, interface);
}

void OACallGraphAnnotationMap::dumpdot(std::ostream & os) {
  //   output graph in DOT form
  OA::OA_ptr<OA::OutputBuilder> dot_builder; dot_builder = new OA::OutputBuilderDOT;
  m_call_graph->configOutput(dot_builder);
  m_call_graph->output(*R_Analyst::get_instance()->get_interface());
}


} // namespace RAnnot
