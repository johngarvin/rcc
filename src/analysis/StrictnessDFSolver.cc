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

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/Interface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/AnalysisResults.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/StrictnessDFSet.h>
#include <analysis/Utils.h>
#include <analysis/VarRefSet.h>

#include "StrictnessDFSolver.h"

using namespace OA;
using namespace HandleInterface;
using namespace RAnnot;

class OA::CFG::Interface;

namespace Strictness {

/// Initialize as a forward data flow problem
StrictnessDFSolver::StrictnessDFSolver(OA_ptr<R_IRInterface> ir)
  : DataFlow::CFGDFProblem(DataFlow::Forward), m_ir(ir)
{}

/// Perform the data flow analysis.
void StrictnessDFSolver::perform_analysis(ProcHandle proc, OA_ptr<CFG::Interface> cfg) {
  m_proc = proc;
  m_cfg = cfg;
  SEXP formals = CAR(fundef_args_c(make_sexp(m_proc)));
  m_formal_args = new DFSet;
  m_formal_args->insert_varset(R_VarRefSet::refs_from_arglist(formals));
}

/// Print out a representation of the in and out sets for each CFG node.
void StrictnessDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

/// Print out a representation of the in and out sets for each CFG node.
void StrictnessDFSolver::dump_node_maps(ostream & os) {
  OA_ptr<DataFlow::DataFlowSet> df_in_set, df_out_set;
  OA_ptr<DFSet> in_set, out_set;
  OA_ptr<CFG::Interface::NodesIterator> ni = m_cfg->getNodesIterator();
  
  for ( ; ni->isValid(); ++*ni) {
    OA_ptr<CFG::Interface::Node> n = ni->current();
    df_in_set = mNodeInSetMap[n];
    df_out_set = mNodeOutSetMap[n];
    in_set = df_in_set.convert<DFSet>();
    out_set = df_out_set.convert<DFSet>();
    os << "CFG NODE #" << n->getId() << ":\n";
    os << "IN SET:\n";
    in_set->dump(os, m_ir);
    os << "OUT SET:\n";
    out_set->dump(os, m_ir);
  }
}

  // ----- Implementing the callbacks for CFGDFProblem -----

/// Initialize TOP as an empty set
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::initializeTop() {
  OA_ptr<DFSet> top;
  top = new DFSet;
  return top;
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::initializeBottom() {
  assert(0);
}

/// Creates in and out DFSets and stores them in mNodeInSetMap and
/// mNodeOutSetMap.
void StrictnessDFSolver::initializeNode(OA_ptr<CFG::Interface::Node> n) {
  if (n.ptrEqual(m_cfg->getEntry())) {
    mNodeInSetMap[n] = m_formal_args->clone();
  } else {
    mNodeInSetMap[n] = new DFSet;
  }
  mNodeOutSetMap[n] = new DFSet;
}

/// Meet function merges info from predecessors. CFGDFProblem says: OK
/// to modify set1 and return it as result, because solver only passes
/// a tempSet in as set1
///
/// For strictness, this is a must problem, so the meet is the set
/// intersection.
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::
meet(OA_ptr<DataFlow::DataFlowSet> set1_orig, OA_ptr<DataFlow::DataFlowSet> set2_orig) {
  OA_ptr<DFSet> set1; set1 = set1_orig.convert<DFSet>();
  OA_ptr<DFSet> set2; set2 = set2_orig.convert<DFSet>();
  return set1->intersect(set2);
}

/// Transfer function adds data given a statement. CFGDFProblem says:
/// OK to modify in set and return it again as result because solver
/// clones the BB in sets
///
/// For strictness analysis, we add must-uses of formal arguments.
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::
transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs, StmtHandle stmt_handle) {
  OA_ptr<DFSet> in; in = in_dfs.convert<DFSet>();
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(stmt_handle));
  ExpressionInfo::const_var_iterator var_iter;
  for(var_iter = annot->begin_vars(); var_iter != annot->end_vars(); ++var_iter) {
    OA_ptr<R_VarRef> mention; mention = new R_BodyVarRef((*var_iter)->getMention_c());
    if (m_formal_args->includes_name(mention) &&
	(*var_iter)->getUseDefType() == Var::Var_USE &&
	(*var_iter)->getMayMustType() == Var::Var_MUST)
    {
      in->insert(mention);
    }
  }
  return in;
}

}  // end namespace Strictness
