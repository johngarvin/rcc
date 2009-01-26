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

// File: DebutDFSolver.cc
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

#include "DebutDFSolver.h"

using namespace RAnnot;
using namespace OA;
using namespace HandleInterface;

typedef DefaultDFSet DFSet;

DebutDFSolver::DebutDFSolver(OA_ptr<R_IRInterface> ir)
  : m_ir(ir), m_fact(VarRefFactory::get_instance())
{}

DebutDFSolver::~DebutDFSolver()
{}

/// Perform the data flow analysis. The main data flow analysis really
/// solves the "must have been mentioned" problem. In a post-pass, use
/// this information to find the debuts. If a name is mentioned in the
/// current statement and does not appear in the has-been-mentioned
/// set, then it's a debut.
OA_ptr<NameMentionMultiMap> DebutDFSolver::perform_analysis(ProcHandle proc, OA_ptr<CFG::CFGInterface> cfg) {
  OA_ptr<CFG::NodeInterface> node;
  UseVar * use;
  DefVar * def;
  StmtHandle stmt;

  m_proc = proc;
  m_cfg = cfg;

  // solve as a forward data flow problem
  m_solver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Forward, *this);
  m_solver->solve(cfg, DataFlow::ITERATIVE);

  // now find the debuts for each name
  OA_ptr<NameMentionMultiMap> debut_map; debut_map = new NameMentionMultiMap();
  
  CFG_FOR_EACH_NODE(m_cfg, node) {
    OA_ptr<DFSet> in_set = m_solver->getInSet(node).convert<DFSet>();
    NODE_FOR_EACH_STATEMENT(node, stmt) {
      // for each mention
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(stmt));
      assert(stmt_annot != 0);
      EXPRESSION_FOR_EACH_USE(stmt_annot, use) {
	OA_ptr<R_BodyVarRef> ref; ref = m_fact->make_body_var_ref(use->getMention_c());
	if (! in_set->member(ref)) {
	  debut_map->insert(std::make_pair(ref->get_sexp(), use->getMention_c()));
	}
      }
      EXPRESSION_FOR_EACH_DEF(stmt_annot, def) {
	OA_ptr<R_BodyVarRef> ref; ref = m_fact->make_body_var_ref(def->getMention_c());
	if (! in_set->member(ref)) {
	  debut_map->insert(std::make_pair(ref->get_sexp(), def->getMention_c()));
	}
      }
      in_set = transfer(in_set, stmt).convert<DFSet>();
    }  // next statement
  }  // next CFG node

  return debut_map;
}

// ----- debugging -----

void DebutDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

void DebutDFSolver::dump_node_maps(ostream &os) {
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
OA_ptr<DataFlow::DataFlowSet> DebutDFSolver::initializeTop() {
  if (m_top.ptrEqual(NULL)) {
    m_top = new DFSet();
    FuncInfo * func = getProperty(FuncInfo, make_sexp(m_proc));
    PROC_FOR_EACH_MENTION(func, mi) {
      OA_ptr<DFSetElement> mention; mention = m_fact->make_body_var_ref((*mi)->getMention_c());
      m_top->insert(mention);
    }
  }
  return m_top;
}

/// Not used.
OA_ptr<DataFlow::DataFlowSet> DebutDFSolver::initializeBottom() {
  assert(0);
}

/// Initializes in and out sets for each node with DFSets. On
/// procedure entry, no variables have been mentioned, so initialize
/// with the empty set. On entry to all other procedures, initialize
/// to TOP (the set of all variables) so that meets (intersections)
/// will not erase other sets.
OA_ptr<DataFlow::DataFlowSet> DebutDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  if (n.ptrEqual(m_cfg->getEntry())) {
    OA_ptr<DFSet> dfset; dfset = new DFSet;
    return dfset.convert<DataFlow::DataFlowSet>();  // upcast
  } else {
    return m_top->clone();
  }
}

OA_ptr<DataFlow::DataFlowSet> DebutDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  return m_top->clone();
}

/// Meet function. Intersect the sets; a variable is in the
/// must-have-been-mentioned set only if it has been mentioned on all
/// incoming paths.
///
/// Note: base class CFGDFProblem says: OK to modify set1 and return
/// it as result, because solver only passes a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet>
DebutDFSolver::meet(OA_ptr<DataFlow::DataFlowSet> set1_orig, OA_ptr<DataFlow::DataFlowSet> set2_orig) {
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
DebutDFSolver::transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs, StmtHandle stmt_handle) {
  UseVar * use;
  DefVar * def;
  OA_ptr<DFSet> in; in = in_dfs.convert<DFSet>();
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(stmt_handle));
  // only local name can be debuts
  //
  // What about TOP? Since we are solving the
  // must-have-been-mentioned problem, if it might be local or free
  // we conservatively say it hasn't been mentioned.
  EXPRESSION_FOR_EACH_USE(annot, use) {
    if (use->getScopeType() == Locality::Locality_LOCAL) {
      OA_ptr<R_VarRef> mention; mention = m_fact->make_body_var_ref(use->getMention_c());
      in->insert(mention);
    }
  }
  EXPRESSION_FOR_EACH_DEF(annot, def) {
    if (def->getScopeType() == Locality::Locality_LOCAL) {
      OA_ptr<R_VarRef> mention; mention = m_fact->make_body_var_ref(def->getMention_c());
      in->insert(mention);
    }
  }
  return in;  
}
