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

// File: ReturnedDFSolver.cc

// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/DataFlow/ICFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/ICFGDFSolver.hpp>

#include <support/Debug.h>

#include <analysis/Analyst.h>
#include <analysis/DefVar.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/NameBoolDFSet.h>

#include "ReturnedDFSolver.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

typedef NameBoolDFSet MyDFSet;

static bool debug;

ReturnedDFSolver::ReturnedDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir)
{
  RCC_DEBUG("RCC_ReturnedDFSolver", debug);
}

ReturnedDFSolver::~ReturnedDFSolver()
{}

OA_ptr<std::map<SEXP, bool> > ReturnedDFSolver::perform_analysis(OA_ptr<ICFG::ICFGInterface> icfg)
{
  m_icfg = icfg;

  // use OA to solve data flow problem
  DataFlow::ICFGDFSolver solver(DataFlow::ICFGDFSolver::Backward, *this);
  solver.solve(icfg, DataFlow::ITERATIVE);
}

// ----- debugging -----

void ReturnedDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

void ReturnedDFSolver::dump_node_maps(std::ostream &os) {
  m_solver->dump(os, m_ir);
  // OA_ptr<DataFlow::DataFlowSet> df_in_set, df_out_set;
  // OA_ptr<MyDFSet> in_set, out_set;
  // OA_ptr<ICFG::NodesIteratorInterface> ni = m_icfg->getICFGNodesIterator();
  // for ( ; ni->isValid(); ++*ni) {
  //   OA_ptr<ICFG::NodeInterface> n = ni->current().convert<ICFG::NodeInterface>();
  //   df_in_set = m_solver->getInSet(n);
  //   df_out_set = m_solver->getOutSet(n);
  //   in_set = df_in_set.convert<MyDFSet>();
  //   out_set = df_out_set.convert<MyDFSet>();
  //   os << "CFG NODE #" << n->getId() << ":\n";
  //   os << "IN SET:\n";
  //   in_set->dump(os, m_ir);
  //   os << "OUT SET:\n";
  //   out_set->dump(os, m_ir);
  // }
}

// ----- callbacks for ICFGDFProblem -----

OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::initializeTop() {
  FuncInfo * fi;
  Var * m;
  DefVar * def;

  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_MENTION(fi, m) {
      if ((def = dynamic_cast<DefVar *>(*m)) != 0) {
	OA_ptr<NameBoolDFSet::NameBoolPair> element;
	element = new NameBoolDFSet::NameBoolPair(def->getName(), false);
	m_top->insert(element);
      }
    }
  }
  return m_top;
}

OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::initializeNodeIN(OA_ptr<ICFG::NodeInterface> n) {
  return m_top->clone();
}

OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::initializeNodeOUT(OA_ptr<ICFG::NodeInterface> n) {
  return m_top->clone();
}

/// ICFGDFProblem says: OK to modify set1 and return it as result, because solver
/// only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::meet(OA_ptr<DataFlow::DataFlowSet> set1_orig,
						     OA_ptr<DataFlow::DataFlowSet> set2_orig)
{
  OA_ptr<MyDFSet> set1; set1 = set1_orig.convert<MyDFSet>();
  OA_ptr<MyDFSet> set2; set2 = set2_orig.convert<MyDFSet>();
  
  //  return set1->meet(set2).convert<DataFlow::DataFlowSet>(); // TODO
}

//! ICFGDFSolver says: OK to modify in set and return it again as
//! result because solver clones the BB in sets. Proc is procedure
//! that contains the statement.
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::transfer(ProcHandle proc,
							 OA_ptr<DataFlow::DataFlowSet> in_orig,
							 StmtHandle stmt_handle)
{
  OA_ptr<MyDFSet> in; in = in_orig.convert<MyDFSet>();
  SEXP stmt = make_sexp(stmt_handle);

#if 0
  if (is_return_or_implicit_return(stmt)) {
    // return rule
    propagate(true, call_arg(1, stmt));
  } else if (is_assign(stmt) && is_var(assign_lhs(stmt)) && is_local(assign_lhs(stmt))) {
    // v0 = v1 rule
    propagate(in[assign_lhs(stmt)], assign_rhs(stmt));
  } else if (is_assign(stmt) && is_call(assign_rhs(stmt))) {
    // method call rule
    propagate(in[assign_lhs(stmt)], assign_rhs(stmt));
  } else {
    // all other rules: no change
  }
#endif
  return in.convert<DataFlow::DataFlowSet>();  // upcast
}

//! transfer function for the entry node of the given procedure
//! should manipulate incoming data-flow set in any special ways
//! for procedure and return outgoing data-flow set for node
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::entryTransfer(ProcHandle proc,
							      OA_ptr<DataFlow::DataFlowSet> in)
{
} 

//! transfer function for the exit node of the given procedure
//! should manipulate outgoing data-flow set in any special ways
//! for procedure and return incoming data-flow set for node
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::exitTransfer(ProcHandle proc,
							     OA_ptr<DataFlow::DataFlowSet> out)
{
}

//! Propagate a data-flow set from caller to callee
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::callerToCallee(ProcHandle caller,
							       OA_ptr<DataFlow::DataFlowSet> dfset,
							       CallHandle call,
							       ProcHandle callee)
{
}
  
//! Propagate a data-flow set from callee to caller
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::calleeToCaller(ProcHandle callee,
							       OA_ptr<DataFlow::DataFlowSet> dfset,
							       CallHandle call,
							       ProcHandle caller)
{
}

// MMA
//! Propagate a data-flow set from call node to return node
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::callToReturn(ProcHandle caller,
							     OA_ptr<DataFlow::DataFlowSet> dfset,
							     CallHandle call,
							     ProcHandle callee)
{
}


#if 0
// propagate(value, expression): propagate lattice value from expression to subexpressions
void propagate(bool value, SEXP e) {
  if (is_var(e) && is_local(e)) {
    in->replace(e, value);
  } else if (is_call(e)) {
    for (each arg i) {
      if (in[formal i]) {
	propagate(value, call_arg(actual i, e));
      }
    }
  }
}
#endif
