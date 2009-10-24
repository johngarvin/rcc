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

// File: EscapedDFSolver.cc

// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/NameBoolDFSet.h>
#include <analysis/ReturnedDFSolver.h>

#include "EscapedDFSolver.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

typedef NameBoolDFSet MyDFSet;

static bool debug;

static bool assignment_escapes(SEXP e);

EscapedDFSolver::EscapedDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir)
{
  RCC_DEBUG("RCC_EscapedDFSolver", debug);
}

EscapedDFSolver::~EscapedDFSolver()
{}

OA_ptr<MyDFSet> EscapedDFSolver::perform_analysis(ProcHandle proc,
						  OA_ptr<CFG::CFGInterface> cfg)
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

void EscapedDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

void EscapedDFSolver::dump_node_maps(std::ostream &os) {
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

OA_ptr<DataFlow::DataFlowSet> EscapedDFSolver::initializeTop() {
  FuncInfo * fi = getProperty(FuncInfo, make_sexp(m_proc));
  Var * mention;

  VarRefFactory * const fact = VarRefFactory::get_instance();

  PROC_FOR_EACH_MENTION(fi, m) {
    OA_ptr<NameBoolDFSet::NameBoolPair> element;
    element = new NameBoolDFSet::NameBoolPair(fact->make_body_var_ref((*m)->getMention_c()), false);
    m_top->insert(element);
  }
  return m_top;
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> EscapedDFSolver::initializeBottom() {
  assert(0);
}

OA_ptr<DataFlow::DataFlowSet> EscapedDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  return m_top->clone();
}

OA_ptr<DataFlow::DataFlowSet> EscapedDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  return m_top->clone();
}

OA_ptr<DataFlow::DataFlowSet> EscapedDFSolver::meet(OA_ptr<DataFlow::DataFlowSet> set1_orig,
						    OA_ptr<DataFlow::DataFlowSet> set2_orig)
{
  OA_ptr<MyDFSet> set1; set1 = set1_orig.convert<MyDFSet>();
  OA_ptr<MyDFSet> set2; set2 = set2_orig.convert<MyDFSet>();
  
  return set1->meet(set2).convert<DataFlow::DataFlowSet>();
}

bool EscapedDFSolver::escaped_predicate(SEXP call, int arg) {
  assert(is_call(call));
  if (is_library(call_lhs(call)) && !is_library_closure(call_lhs(call))) {
    return PRIMESCAPE(library_value(call_lhs(call)), arg);
  } else {  // unknown call
    return true;
  }
}

OA_ptr<DataFlow::DataFlowSet> EscapedDFSolver::transfer(OA_ptr<DataFlow::DataFlowSet> in_orig,
							StmtHandle stmt_handle)
{
  OA_ptr<MyDFSet> in; in = in_orig.convert<MyDFSet>();
  SEXP cell = make_sexp(stmt_handle);
  SEXP e = CAR(cell);
  FuncInfo * fi = getProperty(FuncInfo, make_sexp(m_proc));
  VarRefFactory * const fact = VarRefFactory::get_instance();

  if (is_assign(e) && assignment_escapes(e)) {
    // upward assignment rule
    MyDFSet::propagate(in, &escaped_predicate, true, assign_rhs_c(e));
  } else if (is_assign(e) && !is_simple_assign(e)) {
    // assignment to field/array/call rule
    MyDFSet::propagate(in, &escaped_predicate, true, assign_rhs_c(e));
  } else if (is_local_assign(e) && is_simple_assign(e)) {
    // v0 = v1 rule and method call rule
    bool escaped_v0 = in->lookup(fact->make_body_var_ref(assign_lhs_c(e)));
    MyDFSet::propagate(in, &escaped_predicate, escaped_v0, assign_rhs_c(e));
    if (escaped_v0) {
      MyDFSet::propagate(in, &ReturnedDFSolver::returned_predicate, true, assign_rhs_c(e));
    }
  } else {
    // all other rules: no change
    ;
  }
  return in.convert<DataFlow::DataFlowSet>();  // upcast
}

// We have an assignment X <- Y or X <<- Y. If X is a symbol, then it
// escapes if we have <<- and doesn't escape if we have <-. If X is a
// more complicated expression, it escapes if and only if the base
// symbol is nonlocal. Example: if the left side of the assignment is
// a[b][c]$foo, then it escapes iff a is nonlocal regardness of the
// kind of arrow.
bool assignment_escapes(SEXP e) {
  if (is_symbol(CAR(assign_lhs_c(e)))) {
    return is_free_assign(e);
  } else {
    SEXP sym_c = assign_lhs_c(e);
    do {
      if (is_struct_field(CAR(sym_c))) {
	sym_c = struct_field_lhs_c(CAR(sym_c));
      } else if (is_subscript(CAR(sym_c))) {
	sym_c = subscript_lhs_c(CAR(sym_c));
      } else if (TYPEOF(CAR(sym_c)) == LANGSXP) {  // e.g., dim(x) <- foo
	sym_c = call_nth_arg_c(CAR(sym_c), 1);
      } else {
	assert(0);
      }
    } while (!is_symbol(CAR(sym_c)));
    return getProperty(Var, CAR(sym_c))->getScopeType() == Locality::Locality_FREE;
  }
}
