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
#include <analysis/OACallGraphAnnotation.h>
#include <analysis/ReturnedDFSolver.h>

#include "EscapedDFSolver.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

typedef ExpressionDFSet MyDFSet;

static bool debug;

static bool assignment_escapes(SEXP e);

EscapedDFSolver::EscapedDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir), m_fact(VarRefFactory::get_instance())
{
  RCC_DEBUG("RCC_EscapedDFSolver", debug);
}

EscapedDFSolver::~EscapedDFSolver()
{}

OA_ptr<MyDFSet> EscapedDFSolver::perform_analysis(ProcHandle proc,
						  OA_ptr<CFG::CFGInterface> cfg,
						  OA_ptr<MyDFSet> returned)
{
  OA_ptr<MyDFSet> top; top = new MyDFSet();
  return perform_analysis(proc, cfg, returned, top);
}


OA_ptr<MyDFSet> EscapedDFSolver::perform_analysis(ProcHandle proc,
						  OA_ptr<CFG::CFGInterface> cfg,
						  OA_ptr<MyDFSet> returned,
						  OA_ptr<MyDFSet> in_set)
{
  m_proc = proc;
  m_cfg = cfg;
  m_returned = returned;
  m_top = new MyDFSet();
  m_func_info = getProperty(FuncInfo, HandleInterface::make_sexp(proc));

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
  m_top = new MyDFSet();
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
  OA_ptr<MyDFSet> out;
  SEXP cell = make_sexp(stmt_handle);
  SEXP e = CAR(cell);

#if 0
  if (is_assign(e) && assignment_escapes(e)) {
    // upward assignment rule
    MyDFSet::propagate_rhs(in, &escaped_predicate, true, assign_rhs_c(e));
  } else if (is_assign(e) && !is_simple_assign(e)) {
    // assignment to field/array/call rule
    MyDFSet::propagate_rhs(in, &escaped_predicate, true, assign_rhs_c(e));
  } else if (is_local_assign(e) && is_simple_assign(e)) {
    // v0 = v1 rule and method call rule
    bool escaped_v0 = in->lookup(fact->make_body_var_ref(assign_lhs_c(e)));
    MyDFSet::propagate_rhs(in, &escaped_predicate, escaped_v0, assign_rhs_c(e));
    if (escaped_v0) {
      MyDFSet::propagate_rhs(in, &ReturnedDFSolver::returned_predicate, true, assign_rhs_c(e));
    }
  } else {
    // all other rules: no change
    ;
  }
  return in.convert<DataFlow::DataFlowSet>();  // upcast
#endif
  out = esc(cell, false, in);
  return out.convert<DataFlow::DataFlowSet>();
}

OA_ptr<MyDFSet> EscapedDFSolver::esc(SEXP cell, bool b, OA_ptr<MyDFSet> old_c) {
  OA_ptr<MyDFSet> s;
  OA_ptr<MyDFSet> new_c; new_c = old_c->clone().convert<MyDFSet>();
  if (cell == R_NilValue) {
    return new_c;
  }
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  if (m_func_info->is_return(cell) && !is_explicit_return(cell)) {
    b = false;
    // continue with current expression
  }
  if (b) {
    new_c->insert(cell);
    if (TYPEOF(e) == SYMSXP) {
      new_c->insert(e);
    }
  }

  if (is_explicit_return(e)) {
    return esc(m_func_info->return_value_c(cell), false, new_c);
  } else if (is_fundef(e)) {
    return new_c;
    // will be handled separately
  } else if (is_symbol(e)) {
    return new_c;
  } else if (is_const(e)) {
    return new_c;
  } else if (is_struct_field(e)) {
    return esc(struct_field_lhs_c(e), false, new_c);
  } else if (is_curly_list(e)) {
    return esc_curly_list(curly_body(e), b, new_c);
  } else if (is_assign(e) && is_struct_field(CAR(assign_lhs_c(e)))) {
    return esc(struct_field_lhs_c(CAR(assign_lhs_c(e))), false, new_c)->meet(esc(assign_rhs_c(e), true, new_c));
  } else if (is_assign(e) && is_simple_subscript(CAR(assign_lhs_c(e)))) {
    OA_ptr<MyDFSet> out; out = esc(subscript_first_sub_c(CAR(assign_lhs_c(e))), b, new_c);
    if (is_local_assign(e)) {
      out = out->meet(esc(assign_rhs_c(e), b, new_c));
    } else {
      out = out->meet(esc(assign_rhs_c(e), true, new_c));
      out->insert(cell);
    }
    return out;
  } else if (is_simple_assign(e) && is_local_assign(e)) {
    return esc(assign_rhs_c(e), new_c->lookup(CAR(assign_lhs_c(e))), new_c);
  } else if (is_simple_assign(e) && is_free_assign(e)) {
    return esc(assign_rhs_c(e), true, new_c);
  } else if (is_call(e)) {
    if (!is_symbol(call_lhs(e))) {
      return conservative_call(e, new_c);
    }
    OACallGraphAnnotation * cga = getProperty(OACallGraphAnnotation, e);
    if (cga == 0) {
      if (is_library(call_lhs(e)) && !is_library_closure(call_lhs(e))) {
	// call to library procedure
	SEXP lib = library_value(call_lhs(e));
	s = new_c->clone().convert<MyDFSet>();
	int i = 1;
	for(SEXP arg_c = call_args(e); arg_c != R_NilValue; arg_c = CDR(arg_c)) {
	  bool arg_ret = (PRIMESCAPE(lib, i) || (b && PRIMPOINTS(lib, i)));
	  s = s->meet(esc(arg_c, arg_ret, new_c));
	  i++;
	}
	return s;
      } else {
	// TODO: handle wrappers around .Internal here
	return conservative_call(e, new_c);

	// temporary: call all closure libraries non-escaping
	//return new_c;
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
	bool arg_esc = (new_c->lookup(callee->get_arg(i)) ||
			(b && m_returned->lookup(callee->get_arg(i))));
	s = s->meet(esc(arg_c, arg_esc, new_c));
	i++;
      }
      return s;
    }
  } else {
    assert(0);
  }
}

OA_ptr<MyDFSet> EscapedDFSolver::esc_curly_list(SEXP e, bool b, OA_ptr<MyDFSet> c) {
  if (e == R_NilValue) {
    return c;
  } else if (CDR(e) == R_NilValue) {
    return esc(CAR(e), b, c);
  } else {
    OA_ptr<MyDFSet> cprime; cprime = esc_curly_list(CDR(e), b, c);
    return esc(CAR(e), false, cprime);
  }
}


OA_ptr<MyDFSet> EscapedDFSolver::make_universal() {
  OA_ptr<MyDFSet> all; all = m_top->clone().convert<MyDFSet>();
  all->setUniversal();
  return all;
}

OA_ptr<MyDFSet> EscapedDFSolver::conservative_call(SEXP e, OA_ptr<MyDFSet> in) {
  OA_ptr<MyDFSet> s; s = in->clone().convert<MyDFSet>();
  for(SEXP arg_c = call_args(e); arg_c != R_NilValue; arg_c = CDR(arg_c)) {
    s = s->meet(esc(arg_c, true, in));
  }
  return s;
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
    return getProperty(Var, sym_c)->getScopeType() == Locality::Locality_FREE;
  }
}
