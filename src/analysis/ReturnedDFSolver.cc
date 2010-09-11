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

#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/DefVar.h>
#include <analysis/ExpressionDFSet.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/OACallGraphAnnotation.h>
#include <analysis/Utils.h>
#include <analysis/VarRefFactory.h>

#include "ReturnedDFSolver.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

typedef ExpressionDFSet MyDFSet;
typedef ExpressionDFSet::ExpressionDFSetIterator MyDFSetIterator;

static bool debug;

ReturnedDFSolver::ReturnedDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir), m_fact(VarRefFactory::instance())
{
  RCC_DEBUG("RCC_ReturnedDFSolver", debug);
}

ReturnedDFSolver::~ReturnedDFSolver()
{}

OA_ptr<MyDFSet> ReturnedDFSolver::perform_analysis(ProcHandle proc,
						   OA_ptr<CFG::CFGInterface> cfg)
{
  OA_ptr<MyDFSet> top; top = new MyDFSet();
  return perform_analysis(proc, cfg, top);
}

OA_ptr<MyDFSet> ReturnedDFSolver::perform_analysis(ProcHandle proc,
						   OA_ptr<CFG::CFGInterface> cfg,
						   OA_ptr<MyDFSet> in_set)
{
  m_proc = proc;
  m_cfg = cfg;
  m_top = new MyDFSet();
  m_in = in_set;
  m_func_info = getProperty(FuncInfo, HandleInterface::make_sexp(proc));

  // use OA to solve data flow problem
  m_solver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Backward, *this);
  OA_ptr<DataFlow::DataFlowSet> entry_dfs = m_solver->solve(m_cfg, DataFlow::ITERATIVE);
  OA_ptr<MyDFSet> entry = entry_dfs.convert<MyDFSet>();
  return entry;
}

// ----- debugging -----

void ReturnedDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

void ReturnedDFSolver::dump_node_maps(std::ostream &os) {
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

OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::initializeTop() {
  m_top = new MyDFSet();
  return m_top;
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::initializeBottom() {
  assert(0);
}

OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  return m_top->clone();
}

OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  if (n.ptrEqual(m_cfg->getExit())) {
    return m_in->clone();
  } else {
    return m_top->clone();
  }
}

/// CFGDFProblem says: OK to modify set1 and return it as result, because solver
/// only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::meet(OA_ptr<DataFlow::DataFlowSet> set1_orig,
						     OA_ptr<DataFlow::DataFlowSet> set2_orig)
{
  OA_ptr<MyDFSet> set1; set1 = set1_orig.convert<MyDFSet>();
  OA_ptr<MyDFSet> set2; set2 = set2_orig.convert<MyDFSet>();
  
  OA_ptr<MyDFSet> out; out = set1->meet(set2);
  return out.convert<DataFlow::DataFlowSet>();
}

// Given a call (library or unknown) and an argument, returns true if
// the return value of the procedure points to the argument
bool ReturnedDFSolver::returned_predicate(SEXP call, int arg) {
  assert(is_call(call));
  if (is_library(call_lhs(call)) && !is_library_closure(call_lhs(call))) {
    return PRIMPOINTS(library_value(call_lhs(call)), arg);
  } else {  // unknown call
    return true;
  }
}

/// CFGDFSolver says: OK to modify in set and return it again as
/// result because solver clones the BB in sets.
OA_ptr<DataFlow::DataFlowSet> ReturnedDFSolver::transfer(OA_ptr<DataFlow::DataFlowSet> in_orig,
							 StmtHandle stmt)
{
  OA_ptr<MyDFSet> in; in = in_orig.convert<MyDFSet>();
  OA_ptr<MyDFSet> out;
  SEXP cell = make_sexp(stmt);
  out = ret(cell, false, in);
  return out.convert<DataFlow::DataFlowSet>();  // upcast
}

OA_ptr<MyDFSet> ReturnedDFSolver::ret(SEXP cell, bool b, OA_ptr<MyDFSet> old_c) {
  OA_ptr<MyDFSet> s;
  OA_ptr<MyDFSet> new_c; new_c = old_c->clone().convert<MyDFSet>();
  if (cell == R_NilValue) {
    return new_c;
  }
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  if (m_func_info->is_return(cell) && !is_explicit_return(e)) {
    b = true;
    // continue with current expression
  }
  if (b) {
    new_c->insert(cell);
    if (TYPEOF(e) == SYMSXP) {
      new_c->insert(e);
    }
  }

  if (is_explicit_return(e)) {
    new_c->insert(cell);
    if (TYPEOF(CADR(e)) == SYMSXP) {   // "return(x)" where x is some symbol
      new_c->insert(CADR(e));
    }
    return ret(m_func_info->return_value_c(cell), true, new_c);
  } else if (is_fundef(e)) {
    return new_c;
    // will be handled separately
  } else if (is_symbol(e)) {
    return new_c;
  } else if (is_const(e)) {
    return new_c;
  } else if (is_struct_field(e)) {
    return ret(struct_field_lhs_c(e), false, new_c);
  } else if (is_curly_list(e)) {
    return ret_curly_list(curly_body(e), b, new_c);
  } else if (is_assign(e) && is_struct_field(CAR(assign_lhs_c(e)))) {
    return ret(struct_field_lhs_c(CAR(assign_lhs_c(e))), false, new_c)->meet(ret(assign_rhs_c(e), false, new_c));
  } else if (is_simple_assign(e) && is_local_assign(e)) {
    bool symbol_returns = new_c->lookup(CAR(assign_lhs_c(e)));
    return ret(assign_rhs_c(e), symbol_returns, new_c);
  } else if (is_simple_assign(e) && is_free_assign(e)) {
    return ret(assign_rhs_c(e), false, new_c);
  } else if (is_call(e)) {
    if (!is_symbol(call_lhs(e))) {
      // conservative: each formal argument may be returned
      return conservative_call(e, new_c);
    }
    OACallGraphAnnotation * cga = getProperty(OACallGraphAnnotation, e);
    if (cga == 0) {
      if (is_library(call_lhs(e)) && !is_library_closure(call_lhs(e))) {
	// call to library procedure
	s = new_c->clone().convert<MyDFSet>();
	int i = 1;
	for(SEXP arg_c = call_args(e); arg_c != R_NilValue; arg_c = CDR(arg_c)) {
	  bool arg_ret = PRIMPOINTS(library_value(call_lhs(e)), i);
	  s = s->meet(ret(arg_c, (b && arg_ret), new_c));
	  i++;
	}
	return s;
      } else {
	// TODO: handle wrappers around .Internal calls here
	return conservative_call(e, new_c);

	// temporary: calling all library closures non-returning
	//turn new_c;
      }
    } else {
      ProcHandle proc = cga->get_singleton_if_exists();
      if (proc == ProcHandle(0)) {
	return conservative_call(e, new_c);
      }
      FuncInfo * callee = getProperty(FuncInfo, HandleInterface::make_sexp(proc));
      s = new_c->clone().convert<MyDFSet>();
      int i = 1;
      for(SEXP arg_c = call_args(e); arg_c != R_NilValue; arg_c = CDR(arg_c)) {
	s = s->meet(ret(arg_c, (b && new_c->lookup(callee->get_arg(i))), new_c));
	i++;
      }
      return s;
    }
  } else {
    assert(0);
  }
}

OA_ptr<MyDFSet> ReturnedDFSolver::ret_curly_list(SEXP e, bool b, OA_ptr<MyDFSet> c) {
  if (e == R_NilValue) {
    return c;
  } else if (CDR(e) == R_NilValue) {
    return ret(e, b, c);
  } else {
    OA_ptr<MyDFSet> cprime; cprime = ret_curly_list(CDR(e), b, c);
    return ret(e, false, cprime);
  }
}

OA_ptr<MyDFSet> ReturnedDFSolver::make_universal_set() {
  OA_ptr<MyDFSet> all; all = m_top->clone().convert<MyDFSet>();
  all->setUniversal();
  return all;  
}

OA_ptr<MyDFSet> ReturnedDFSolver::conservative_call(SEXP e, OA_ptr<MyDFSet> in) {
  OA_ptr<MyDFSet> s; s = in->clone().convert<MyDFSet>();
  for(SEXP arg_c = call_args(e); arg_c != R_NilValue; arg_c = CDR(arg_c)) {
    s = s->meet(ret(arg_c, true, in));
  }
  return s;
}
