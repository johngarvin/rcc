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

// File: FirstMentionDFSolver.cc
//
// Implements the general OpenAnalysis CFG data flow problem. For each
// local variable in a given procedure, finds a set of mentions: a
// mention is in the set if and only if it's the first mention of that
// variable on some path. It's similar to the MUST-KILL data flow
// problem, except that a variable can be "killed" by a use or a def,
// not just a def. Useful for discovering where arguments (which are
// lazy in R) may be evaluated. For this problem, formal arguments do
// not count as defs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/DataFlow/CFGDFSolver.hpp>

#include <analysis/AnalysisResults.h>
#include <analysis/DefaultDFSet.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/IRInterface.h>
#include <analysis/HandleInterface.h>
#include <analysis/NameMentionMultiMap.h>
#include <analysis/PropertySet.h>
#include <analysis/VarRef.h>
#include <analysis/Var.h>

#include "FirstMentionDFSolver.h"

using namespace RAnnot;
using namespace OA;
using namespace HandleInterface;

typedef DefaultDFSet DFSet;

FirstMentionDFSolver::FirstMentionDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir), m_fact(VarRefFactory::get_instance())
{}

FirstMentionDFSolver::~FirstMentionDFSolver()
{}

/// Perform the data flow analysis. The main data flow analysis really
/// solves the "must have been mentioned" problem. In a post-pass, use
/// this information to find the first mentions. If a name is
/// mentioned in the current statement and does not appear in the
/// has-been-mentioned set, then it's a first mention.
OA_ptr<NameMentionMultiMap> FirstMentionDFSolver::perform_analysis(ProcHandle proc, OA_ptr<CFG::CFGInterface> cfg) {
  m_proc = proc;
  m_cfg = cfg;

  // solve as a forward data flow problem
  m_solver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Forward, *this);
  m_solver->solve(cfg, DataFlow::ITERATIVE);

  // now find the first mentions for each variable
  OA_ptr<NameMentionMultiMap> first_mention_map; first_mention_map = new NameMentionMultiMap();
  // for each CFG node
  OA_ptr<CFG::NodesIteratorInterface> ni; ni = cfg->getCFGNodesIterator();
  for ( ; ni->isValid(); ++*ni) {
    OA_ptr<CFG::Node> n = ni->current().convert<CFG::Node>();
    OA_ptr<DFSet> in_set = m_solver->getInSet(n).convert<DFSet>();
    // for each statement
    OA_ptr<CFG::NodeStatementsIteratorInterface> si; si = n->getNodeStatementsIterator();
    for ( ; si->isValid(); ++*si) {
      // for each mention
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(si->current()));
      assert(stmt_annot != 0);
      ExpressionInfo::const_var_iterator mi;
      for (mi = stmt_annot->begin_vars(); mi != stmt_annot->end_vars(); ++mi) {
	OA_ptr<R_BodyVarRef> ref; ref = m_fact->make_body_var_ref((*mi)->getMention_c());
	if (! in_set->member(ref)) {
	  first_mention_map->insert(std::make_pair(ref->get_sexp(), (*mi)->getMention_c()));
	}
      }  // next mention
      in_set = transfer(in_set, si->current()).convert<DFSet>();
    }  // next statement
  }  // next CFG node

  return first_mention_map;
}

// ----- debugging -----

void FirstMentionDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

void FirstMentionDFSolver::dump_node_maps(ostream &os) {
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

// ----- callbacks for CFGDFProblem: initialization, meet, transfer -----

/// Initialize TOP as the set of all variables mentioned
OA_ptr<DataFlow::DataFlowSet> FirstMentionDFSolver::initializeTop() {
  if (m_top.ptrEqual(NULL)) {
    m_top = new DFSet();
    FuncInfo * func = getProperty(FuncInfo, make_sexp(m_proc));
    FuncInfo::mention_iterator mi;
    for (mi = func->begin_mentions(); mi != func->end_mentions(); mi++) {
      OA_ptr<DFSetElement> mention; mention = m_fact->make_body_var_ref((*mi)->getMention_c());
      m_top->insert(mention);
    }
  }
  return m_top;
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> FirstMentionDFSolver::initializeBottom() {
  assert(0);
}

/// Initializes in and out sets for each node with DFSets. On
/// procedure entry, no variables have been mentioned, so initialize
/// with the empty set. On entry to all other procedures, initialize
/// to TOP (the set of all variables) so that meets (intersections)
/// will not erase other sets.
OA_ptr<DataFlow::DataFlowSet> FirstMentionDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  if (n.ptrEqual(m_cfg->getEntry())) {
    OA_ptr<DFSet> dfset; dfset = new DFSet;
    return dfset.convert<DataFlow::DataFlowSet>();  // upcast
  } else {
    return m_top->clone();
  }
}

OA_ptr<DataFlow::DataFlowSet> FirstMentionDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  return m_top->clone();
}

/// Meet function. Intersect the sets; a variable is in the
/// must-have-been-mentioned set only if it has been mentioned on all
/// incoming paths.
///
/// Note: base class CFGDFProblem says: OK to modify set1 and return
/// it as result, because solver only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet>
FirstMentionDFSolver::meet(OA_ptr<DataFlow::DataFlowSet> set1_orig, OA_ptr<DataFlow::DataFlowSet> set2_orig) {
  // downcast sets from interface DataFlowSet to concrete class DFSet
  OA_ptr<DFSet> set1; set1 = set1_orig.convert<DFSet>();
  OA_ptr<DFSet> set2; set2 = set2_orig.convert<DFSet>();
  // perform the intersection
  return set1->intersect(set2);
}

/// Transfer function; the effect of a statement
/// Union each mention into the set
///
/// Note: base class CFGDFProblem says: OK to modify in set and return
/// it again as result because solver clones the BB in sets
OA_ptr<DataFlow::DataFlowSet> 
FirstMentionDFSolver::transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs, StmtHandle stmt_handle) {
  OA_ptr<DFSet> in; in = in_dfs.convert<DFSet>();
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(stmt_handle));
  ExpressionInfo::const_var_iterator var_iter;
  for(var_iter = annot->begin_vars(); var_iter != annot->end_vars(); ++var_iter) {
    // only local variables can be first mentions
    //
    // What about TOP? Since we are solving the
    // must-have-been-mentioned problem, if it might be local or free
    // we conservatively say it hasn't been mentioned.
    if ((*var_iter)->getScopeType() == Locality_LOCAL) {
      OA_ptr<R_VarRef> mention; mention = m_fact->make_body_var_ref((*var_iter)->getMention_c());
      in->insert(mention);
    }
  }
  return in;  
}
