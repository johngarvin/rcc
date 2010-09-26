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

// File: OEscapeDFSolver.cc

// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>

#include "OEscapeDFSolver.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

typedef ExpressionDFSet MyDFSet;

static bool debug;

OEscapeDFSolver::OEscapeDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir)
{
  RCC_DEBUG("RCC_OEscapeDFSolver", debug);
}

OEscapeDFSolver::~OEscapeDFSolver()
{}

OA_ptr<MyDFSet> OEscapeDFSolver::perform_analysis(ProcHandle proc, OA_ptr<CFG::CFGInterface> cfg)
{
  m_proc = proc;
  m_cfg = cfg;
  m_top = new MyDFSet();
  

  // use OA to solve data flow problem
  m_solver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Backward, *this);
  OA_ptr<DataFlow::DataFlowSet> entry_dfs = m_solver->solve(m_cfg, DataFlow::ITERATIVE);
  OA_ptr<MyDFSet> entry = entry_dfs.convert<MyDFSet>();
  return entry;
}

// ----- debugging -----

void OEscapeDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

void OEscapeDFSolver::dump_node_maps(std::ostream & os) {
  OA_ptr<DataFlow::DataFlowSet> df_in_set, df_out_set;
  OA_ptr<MyDFSet> in_set, out_set;
  OA_ptr<CFG::NodesIteratorInterface> ni = m_cfg->getCFGNodesIterator();
  for ( ; ni->isValid(); ++*ni) {
    OA_ptr<CFG::NodeInterface> n = ni->current().convert<CFG::NodeInterface>();
    df_in_set = m_solver->getInSet(n);
    df_out_set = m_solver->getOutSet(n);
    in_set = df_in_set.convert<MyDFSet>();
    out_set = df_out_set.convert<MyDFSet>();
    os << "CFG NODE #" << n->getId() << ":\n";
    os << "IN SET:\n";
    in_set->dump(os, m_ir);
    os << "OUT SET:\n";
    out_set->dump(os, m_ir);
  }
}

// ----- callbacks for CFGDFProblem -----

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::initializeTop() {
  m_top = new MyDFSet();
  return m_top;
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::initializeBottom() {
  assert(0);
}

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  return m_top->clone();
}

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  return m_top->clone();
}

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::meet(OA_ptr<DataFlow::DataFlowSet> set1_orig,
						    OA_ptr<DataFlow::DataFlowSet> set2_orig)
{
  OA_ptr<MyDFSet> set1; set1 = set1_orig.convert<MyDFSet>();
  OA_ptr<MyDFSet> set2; set2 = set2_orig.convert<MyDFSet>();
  
  return set1->meet(set2).convert<DataFlow::DataFlowSet>();
}

OA_ptr<DataFlow::DataFlowSet> OEscapeDFSolver::transfer(OA_ptr<DataFlow::DataFlowSet> in_orig,
							StmtHandle stmt)
{
  OA_ptr<MyDFSet> in; in = in_orig.convert<MyDFSet>();
  SEXP cell = make_sexp(stmt);
  SEXP e = CAR(cell);
  FuncInfo * fi; fi = getProperty(FuncInfo, make_sexp(m_proc));
  VarRefFactory * const fact = VarRefFactory::instance();

  if (fi->is_return(cell)) {
    // TODO: When add inter: if e is escaped, make mnfresh true for this proc
    //                 in all cases propagate vnfresh to mnfresh
  } else if (is_simple_assign(e) && !is_call(CAR(assign_lhs_c(e)))) {
    in->insert(assign_lhs_c(e));
  } else if (is_assign(e) && !is_simple_assign(e)) {
    // v0.f = v1 rule (may create object in R)
    // creates object. vnfresh stays false TODO: right?
  } else if (is_simple_assign(e) && is_call(CAR(assign_rhs_c(e)))) {
    // method call rule
    const SEXP callee = call_lhs(CAR(assign_rhs_c(e)));
    if (!is_library(callee) || is_library_closure(callee)) {
      in->insert(assign_lhs_c(e));
    }
    // When add inter: propagate mnfresh of callee to vnfresh of v0
  }
  // all other cases: no change
  return in.convert<DataFlow::DataFlowSet>();
}
