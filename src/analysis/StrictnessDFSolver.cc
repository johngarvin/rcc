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

// File: StrictnessDFSolver.cc
//
// Implements the OpenAnalysis CFG data flow problem interface for a
// function to determine whether a function is strict in each of its
// formal arguments. (A function is strict in argument v if there is a
// use of v on all paths.)
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "LocalityDFSolver.h"

using namespace OA;

namespace Strictness {

/// Initialize as a forward data flow problem
StrictnessDFSolver::StrictnessDFSolver(OA_ptr<R_IRInterface> ir)
  : DataFlow::CFGDFProblem(DataFlow::Forward), m_ir:ir
{}

/// Perform the data flow analysis.
void StrictnessDFSolver::perform_analysis(ProcHandle proc, OA_ptr<CFG::Interface> cfg) {
  
}

/// Print out a representation of the in and out sets for each CFG node.
void LocalityDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}


/// Print out a representation of the in and out sets for each CFG node.
void LocalityDFSolver::dump_node_maps() {
}

  // ----- Implementing the callbacks for CFGDFProblem -----

/// Create a set of all variables set to TOP
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::initializeTop() {
  if (m_all_top.ptrEqual(NULL)) {
    initialize_sets();
  }
  return m_all_top->clone();
}

/// Create a set of all variables set to BOTTOM
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::initializeBottom() {
  if (m_all_bottom.ptrEqual(NULL)) {
    initialize_sets();
  }
  return m_all_bottom->clone();
}

/// local function: initialize top, bottom, and initial values
void LocalityDFSolver::initialize_sets() {
  m_all_top = new DFSet;
  m_all_bottom = new DFSet;
  m_entry_values = new DFSet;

  // there's an entry for each formal argument
  SEXP arglist = CAR(fundef_args_c(make_sexp(m_proc)));
  OA_ptr<R_VarRefSet> vs = R_VarRefSet::refs_from_arglist(arglist);
  m_all_top->insert_varset(vs, Locality_TOP);
  m_all_bottom->insert_varset(vs, Locality_BOTTOM);
  m_entry_values->insert_varset(vs, Locality_LOCAL);  
}

/// Creates in and out DFSets and stores them in mNodeInSetMap and
/// mNodeOutSetMap.
void LocalityDFSolver::initializeNode(OA_ptr<CFG::Interface::Node> n) {
  if (n.ptrEqual(m_cfg->getEntry())) {
    mNodeInSetMap[n] = m_entry_values->clone();
  } else {
    mNodeInSetMap[n] = m_all_top->clone();
  }
  mNodeOutSetMap[n] = m_all_top->clone();
}

/// Meet function merges info from predecessors. CFGDFProblem says: OK
/// to modify set1 and return it as result, because solver only passes
/// a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::
meet(OA_ptr<DataFlow::DataFlowSet> set1, OA_ptr<DataFlow::DataFlowSet> set2) {

}

/// Transfer function adds data given a statement. CFGDFProblem says:
/// OK to modify in set and return it again as result because solver
/// clones the BB in sets
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::
transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs, StmtHandle stmt_handle) {
}

}
