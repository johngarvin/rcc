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
// variable, finds a set of mentions: a mention is in the set if and
// only if it's the first mention of that variable on some path. It's
// similar to the MUST-KILL data flow problem, except that a variable
// can be "killed" by a use or a def, not just a def. Useful for
// discovering where arguments (which are lazy in R) may be evaluated.
// For this problem, formal arguments do not count as defs.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/AnalysisResults.h>
#include <analysis/DefaultDFSet.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/IRInterface.h>
#include <analysis/HandleInterface.h>
#include <analysis/NameStmtsMap.h>
#include <analysis/PropertySet.h>
#include <analysis/VarRef.h>
#include <analysis/Var.h>

#include "FirstMentionDFSolver.h"

using namespace RAnnot;
using namespace OA;
using namespace HandleInterface;

typedef DefaultDFSet DFSet;

/// Initialize as a forward data flow problem.
FirstMentionDFSolver::FirstMentionDFSolver(OA_ptr<R_IRInterface> ir)
  : DataFlow::CFGDFProblem(DataFlow::Forward), m_ir(ir)
{}

FirstMentionDFSolver::~FirstMentionDFSolver()
{}

/// Perform the data flow analysis.
OA_ptr<NameStmtsMap> FirstMentionDFSolver::perform_analysis(ProcHandle proc, OA_ptr<CFG::Interface> cfg) {
  m_proc = proc;
  m_cfg = cfg;
  DataFlow::CFGDFProblem::solve(cfg);

  // now collect the first mentions for each variable
  OA_ptr<NameStmtsMap> first_mention_map; first_mention_map = new NameStmtsMap();
  // for each CFG node
  OA_ptr<CFG::Interface::NodesIterator> ni; ni = cfg->getNodesIterator();
  for ( ; ni->isValid(); ++*ni) {
    OA_ptr<DFSet> in_set = mNodeInSetMap[ni->current()].convert<DFSet>();
    // for each statement
    OA_ptr<CFG::Interface::NodeStatementsIterator> si; si = ni->current()->getNodeStatementsIterator();
    for ( ; si->isValid(); ++*si) {
      // for each mention
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(si->current()));
      assert(stmt_annot != 0);
      ExpressionInfo::const_var_iterator mi;
      for (mi = stmt_annot->begin_vars(); mi != stmt_annot->end_vars(); ++mi) {
	OA_ptr<R_BodyVarRef> ref; ref = new R_BodyVarRef((*mi)->getMention_c());
	if (in_set->member(ref)) {
	  first_mention_map->insert(std::make_pair(ref->get_sexp(), si->current()));
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

// ----- callbacks for CFGDFProblem: initialization, meet, transfer -----

/// Initialize TOP as the set of all variables mentioned
OA_ptr<DataFlow::DataFlowSet> FirstMentionDFSolver::initializeTop() {
  if (m_top.ptrEqual(NULL)) {
    m_top = new DFSet();
    FuncInfo * func = getProperty(FuncInfo, make_sexp(m_proc));
    FuncInfo::mention_iterator mi;
    for (mi = func->begin_mentions(); mi != func->end_mentions(); mi++) {
      OA_ptr<DFSetElement> mention; mention = new R_BodyVarRef((*mi)->getMention_c());
      m_top->insert(mention);
    }
  }
  return m_top;
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> FirstMentionDFSolver::initializeBottom() {
  assert(0);
}

/// Initializes mNodeInSetMap and mNodeOutSetMap with DFSets.
void FirstMentionDFSolver::initializeNode(OA_ptr<CFG::Interface::Node> n) {
  // On procedure entry, no variables have been mentioned, so
  // initialize with the empty set. On entry to all other procedures,
  // initialize to TOP (the set of all variables) so that meets will
  // work correctly.
  if (n.ptrEqual(m_cfg->getEntry())) {
    mNodeInSetMap[n] = new DFSet;
  } else {
    mNodeInSetMap[n] = m_top->clone();
  }
  mNodeOutSetMap[n] = m_top->clone();
}

/// Meet function.
// Note: CFGDFProblem says: OK to modify set1 and return it as result,
// because solver only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet>
FirstMentionDFSolver::meet(OA_ptr<DataFlow::DataFlowSet> set1_orig, OA_ptr<DataFlow::DataFlowSet> set2_orig) {
  OA_ptr<DFSet> set1; set1 = set1_orig.convert<DFSet>();
  OA_ptr<DFSet> set2; set2 = set2_orig.convert<DFSet>();
  return set1->intersect(set2);
}

/// Transfer function; the effect of a statement
/// Add each mention to the set
// Note: CFGDFProblem says: OK to modify in set and return it again as
// result because solver clones the BB in sets
OA_ptr<DataFlow::DataFlowSet> 
FirstMentionDFSolver::transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs, StmtHandle stmt_handle) {
  OA_ptr<DFSet> in; in = in_dfs.convert<DFSet>();
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(stmt_handle));
  ExpressionInfo::const_var_iterator var_iter;
  for(var_iter = annot->begin_vars(); var_iter != annot->end_vars(); ++var_iter) {
    OA_ptr<R_VarRef> mention; mention = new R_BodyVarRef((*var_iter)->getMention_c());
    in->insert(mention);
  }
  return in;  
}
