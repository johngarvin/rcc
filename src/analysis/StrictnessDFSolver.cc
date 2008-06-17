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
#include <analysis/NameMentionMultiMap.h>
#include <analysis/NameStmtMultiMap.h>
#include <analysis/StrictnessDFSet.h>
#include <analysis/StrictnessDFSetElement.h>
#include <analysis/StrictnessDFSetIterator.h>
#include <analysis/StrictnessResult.h>
#include <analysis/Utils.h>
#include <analysis/VarRefFactory.h>
#include <analysis/VarRefSet.h>

#include "StrictnessDFSolver.h"

using namespace OA;
using namespace HandleInterface;
using namespace RAnnot;
using namespace Strictness;

// forward declarations

static StrictnessType var_meet(StrictnessType x, StrictnessType y);
static OA_ptr<DFSet> set_meet(OA_ptr<DFSet> set1, OA_ptr<DFSet> set2);

static const bool debug = false;

namespace Strictness {

StrictnessDFSolver::StrictnessDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir), m_var_ref_fact(VarRefFactory::get_instance())
{}

StrictnessDFSolver::~StrictnessDFSolver()
{}

/// Perform the data flow analysis.
OA_ptr<StrictnessResult> StrictnessDFSolver::perform_analysis(ProcHandle proc, OA_ptr<CFG::CFGInterface> cfg) {
  m_proc = proc;
  m_cfg = cfg;
  m_solver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Forward, *this);
  SEXP formals = CAR(fundef_args_c(make_sexp(m_proc)));
  m_formal_args = new DFSet;
  m_formal_args->insert_varset(R_VarRefSet::refs_from_arglist(formals), Strictness_TOP);

  // call the solver, which returns the set of args associated with StrictnessTypes on exit
  OA_ptr<DFSet> args_on_exit = m_solver->solve(cfg, DataFlow::ITERATIVE).convert<DFSet>();
  if (debug) dump_node_maps();

  OA_ptr<NameMentionMultiMap> debut_map = compute_debut_map();
  OA_ptr<NameStmtMultiMap> post_debut_map = compute_post_debut_map(args_on_exit);
  OA_ptr<StrictnessResult> result; result = new StrictnessResult(args_on_exit, debut_map, post_debut_map);
  return result;
}

// Compute debuts. A mention is a debut iff it's a use and it's the
// first mention on some path. In data-flow terms, a mention is a
// debut if it is a use and it is TOP on entry. If it's not TOP on
// entry, that means it has been used or killed prior to this mention.
OA_ptr<NameMentionMultiMap> StrictnessDFSolver::compute_debut_map() {
  Var * var;
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;

  OA_ptr<NameMentionMultiMap> debut_map; debut_map = new NameMentionMultiMap();
  CFG_FOR_EACH_NODE(m_cfg, node) {
    OA_ptr<DFSet> in_set = m_solver->getInSet(node).convert<DFSet>();
    NODE_FOR_EACH_STATEMENT(node, stmt) {
      // for each mention
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(stmt));
      assert(stmt_annot != 0);
      if (debug) {
	std::cout << "Debut: looking at statement:" << std::endl;
	Rf_PrintValue(CAR(make_sexp(stmt)));
      }
      EXPRESSION_FOR_EACH_MENTION(stmt_annot, var) {
	OA_ptr<R_BodyVarRef> ref; ref = m_var_ref_fact->make_body_var_ref(var->getMention_c());
	if (in_set->includes_name(ref) &&
	    var->getUseDefType() == Var::Var_USE &&
	    in_set->find(ref)->get_strictness_type() == Strictness_TOP)
	{
	  debut_map->insert(std::make_pair(ref->get_sexp(), var->getMention_c()));
	  if (debug) {
	    std::cout << "Found debut:" << std::endl;
	    Rf_PrintValue(CAR(var->getMention_c()));
	  }
	}
      }  // next mention
      in_set = transfer(in_set, si->current()).convert<DFSet>();
    }  // next statement
  }  // next CFG node
  return debut_map;
}

// compute post-debut statements (statements for which every path
// from the start to the statement must go through a debut) for each formal
OA_ptr<NameStmtMultiMap> StrictnessDFSolver::compute_post_debut_map(OA_ptr<DFSet> args_on_exit) {
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;

  OA_ptr<NameStmtMultiMap> post_debut_map; post_debut_map = new NameStmtMultiMap();
  // for each formal
  OA_ptr<DFSetIterator> formal_it = args_on_exit->get_iterator();
  for ( ; formal_it->isValid(); ++*formal_it) {
    OA_ptr<R_VarRef> formal; formal = formal_it->current()->get_loc();
    CFG_FOR_EACH_NODE(m_cfg, node) {
      OA_ptr<DFSet> in_set = m_solver->getInSet(node).convert<DFSet>();
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	in_set = transfer(in_set, stmt).convert<DFSet>();
	// if formal is USED at this point, add stmt to map
	if (in_set->find(formal)->get_strictness_type() == Strictness_USED) {
	  post_debut_map->insert(std::make_pair(TAG(formal->get_sexp()), stmt));
	  if (debug) {
	    std::cout << "Found post-debut statement:" << std::endl;
	    Rf_PrintValue(CAR(make_sexp(stmt)));
	  }
	}
      }
    }
  }
  return post_debut_map;
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
  return m_formal_args->clone().convert<DataFlow::DataFlowSet>();
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::initializeBottom() {
  assert(0);
}

/// Initialize with TOP.
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  return m_formal_args->clone().convert<DataFlow::DataFlowSet>();
}

/// Initialize with TOP
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  return m_formal_args->clone().convert<DataFlow::DataFlowSet>();
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
    set1->dump(std::cout, m_ir);
    set2->dump(std::cout, m_ir);
  }
  OA_ptr<DFSet> retval; retval = set_meet(set1, set2);
  return retval.convert<DataFlow::DataFlowSet>();
}

/// Transfer function adds data given a statement. CFGDFProblem says:
/// OK to modify in set and return it again as result because solver
/// clones the BB in sets
///
OA_ptr<DataFlow::DataFlowSet> StrictnessDFSolver::
transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs, StmtHandle stmt_handle) {
  Var * var;
  OA_ptr<DFSet> in; in = in_dfs.convert<DFSet>();
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(stmt_handle));
  EXPRESSION_FOR_EACH_MENTION(annot, var) {
    OA_ptr<R_VarRef> mention; mention = m_var_ref_fact->make_body_var_ref(var->getMention_c());
    
    if (m_formal_args->includes_name(mention) &&
	var->getUseDefType() == Var::Var_DEF)
      {
	in->replace(mention, Strictness_KILLED);
      }
  }
  EXPRESSION_FOR_EACH_MENTION(annot, var) {
    OA_ptr<R_VarRef> mention; mention = m_var_ref_fact->make_body_var_ref(var->getMention_c());
    
    if (m_formal_args->includes_name(mention) &&
	in->find(mention)->get_strictness_type() != Strictness_KILLED &&
	var->getUseDefType() == Var::Var_USE &&
	var->getMayMustType() == Var::Var_MUST)
    {
      in->replace(mention, Strictness_USED);
    }
  }
  return in.convert<DataFlow::DataFlowSet>();
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
OA_ptr<DFSet> set_meet(OA_ptr<DFSet> set1, OA_ptr<DFSet> set2) {
  OA_ptr<DFSet> retval; retval = set1->clone().convert<DFSet>();
  OA_ptr<Strictness::DFSetElement> elem;
  for(OA_ptr<DFSetIterator> it2 = set2->get_iterator(); it2->isValid(); ++*it2) {
    elem = set1->find(it2->current()->get_loc());
    StrictnessType new_type = var_meet(elem->get_strictness_type(), it2->current()->get_strictness_type());
    retval->replace(elem->get_loc(), new_type);
  }
  return retval;
}
