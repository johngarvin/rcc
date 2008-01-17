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
// use of v before a def of v on all paths.)
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/AnalysisResults.h>
#include <analysis/DefaultDFSet.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/StrictnessDFSet.h>
#include <analysis/StrictnessDFSetIterator.h>
#include <analysis/Utils.h>
#include <analysis/VarRefFactory.h>
#include <analysis/VarRefSet.h>

#include "StrictnessDFSolver.h"

using namespace OA;
using namespace HandleInterface;
using namespace RAnnot;

// forward declarations

static StrictnessType var_meet(StrictnessType x, StrictnessType y);
static OA_ptr<Strictness::DFSet> set_meet(OA_ptr<Strictness::DFSet> set1, OA_ptr<Strictness::DFSet> set2);

static const bool debug = true;

namespace Strictness {

StrictnessDFSolver::StrictnessDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir)
{}

StrictnessDFSolver::~StrictnessDFSolver()
{}

/// Perform the data flow analysis.
OA_ptr<DFSet> StrictnessDFSolver::perform_analysis(ProcHandle proc, OA_ptr<CFG::CFGInterface> cfg) {
  m_proc = proc;
  m_cfg = cfg;
  m_solver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Forward, *this);
  SEXP formals = CAR(fundef_args_c(make_sexp(m_proc)));
  m_formal_args = new DFSet;
  m_formal_args->insert_varset(R_VarRefSet::refs_from_arglist(formals), Strictness_TOP);
  OA_ptr<DFSet> retval = m_solver->solve(cfg, DataFlow::ITERATIVE).convert<DFSet>();
  if (debug) dump_node_maps();
  return retval;
}

/// Print out a representation of the in and out sets for each CFG node.
void StrictnessDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

/// Print out a representation of the in and out sets for each CFG node.
void StrictnessDFSolver::dump_node_maps(ostream & os) {
  OA_ptr<DataFlow::DataFlowSet> df_in_set, df_out_set;
  OA_ptr<DFSet> in_set, out_set;

  OA_ptr<CFG::NodesIteratorInterface> ni = m_cfg->getCFGNodesIterator();
  for ( ; ni->isValid(); ++*ni) {
    OA_ptr<CFG::NodeInterface> n = ni->current().convert<CFG::NodeInterface>();
    df_in_set = m_solver->getInSet(n);
    df_out_set = m_solver->getOutSet(n);
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

/// Initialize TOP as the set of formal arg names
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::initializeTop() {
  return m_formal_args->clone();
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::initializeBottom() {
  assert(0);
}

/// Initialize with TOP.
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  return m_formal_args->clone();
}

/// Initialize with TOP
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  return m_formal_args->clone();
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
  if (debug) {
    std::cout << "Intersecting..." << std::endl;
    set1->dump(std::cout);
    set2->dump(std::cout);
  }
  OA_ptr<DFSet> retval; retval = set_meet(set1, set2);
  return retval.convert<DataFlow::DataFlowSet>();
}

/// Transfer function adds data given a statement. CFGDFProblem says:
/// OK to modify in set and return it again as result because solver
/// clones the BB in sets
///
/// For strictness analysis, we add must-uses of formal arguments.
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::
transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs, StmtHandle stmt_handle) {
  VarRefFactory * fact = VarRefFactory::get_instance();
  OA_ptr<DFSet> in; in = in_dfs.convert<DFSet>();
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(stmt_handle));
  ExpressionInfo::const_var_iterator var_iter;
  for(var_iter = annot->begin_vars(); var_iter != annot->end_vars(); ++var_iter) {
    OA_ptr<R_VarRef> mention; mention = fact->make_body_var_ref((*var_iter)->getMention_c());
    
    if (m_formal_args->includes_name(mention) &&
	(*var_iter)->getUseDefType() == Var::Var_DEF)
      {
	in->replace(mention, Strictness_KILLED);
      }
  }
  for(var_iter = annot->begin_vars(); var_iter != annot->end_vars(); ++var_iter) {
    OA_ptr<R_VarRef> mention; mention = fact->make_body_var_ref((*var_iter)->getMention_c());
    
    if (m_formal_args->includes_name(mention) &&
	(*var_iter)->getUseDefType() == Var::Var_USE &&
	(*var_iter)->getMayMustType() == Var::Var_MUST)
    {
      in->replace(mention, Strictness_USED);
    }
  }
  return in;
}

} // end namespace Strictness

//--------------------------------------------------------------------
// Static meet functions
//--------------------------------------------------------------------

// meet function for single lattice element
//    TOP
//   |   \
//   |   USED
//   |   /
//  KILLED
StrictnessType var_meet(StrictnessType x, StrictnessType y) {
  if (x == y) {
    return x;
  } else if (x == Strictness_TOP) {
    return y;
  } else if (y == Strictness_TOP) {
    return x;
  } else {
    return Strictness_KILLED;
  }
}

/// Meet function for two DFSets, using the single-element meet
OA_ptr<Strictness::DFSet> set_meet(OA_ptr<Strictness::DFSet> set1, OA_ptr<Strictness::DFSet> set2) {
  OA_ptr<Strictness::DFSet> retval; retval = set1->clone().convert<Strictness::DFSet>();
  OA_ptr<Strictness::DFSetElement> elem;
  for(OA_ptr<Strictness::DFSetIterator> it2 = set2->get_iterator(); it2->isValid(); ++*it2) {
    elem = set1->find(it2->current()->get_loc());
    StrictnessType new_type = var_meet(elem->get_strictness_type(), it2->current()->get_strictness_type());
    retval->replace(elem->get_loc(), new_type);
  }
}
