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

// File: LocalityDFSolver.cc
//
// Implements the OpenAnalysis CFG data flow problem interface for a
// function to determine whether variable references refer to local or
// free variables.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/AnalysisResults.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/Var.h>
#include <analysis/UseVar.h>
#include <analysis/DefVar.h>
#include <analysis/VarRefFactory.h>

#include <analysis/LocalityDFSet.h>
#include <analysis/LocalityDFSetElement.h>
#include <analysis/LocalityDFSetIterator.h>

#include <support/RccError.h>

#include "LocalityDFSolver.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

namespace Locality {

// forward declarations

static LocalityType var_meet(LocalityType x, LocalityType y);
static OA_ptr<DFSet> meet_use_set(OA_ptr<DFSet> set1, OA_ptr<DFSet> set2);

// static variable for debugging

static const bool debug = false;

/// visitor that returns an R_VarRef of the appropriate type when
/// applied to Var annotation
class MakeVarRefVisitor : private VarVisitor {
public:
  MakeVarRefVisitor() : m_output(), m_fact(VarRefFactory::get_instance()) {}
      
private:
  void visitUseVar(UseVar * uv) {
    m_output = m_fact->make_body_var_ref(uv->getMention_c());
  }
  
  void visitDefVar(DefVar * dv) {
    switch(dv->getSourceType()) {
    case DefVar::DefVar_ASSIGN:
      m_output = m_fact->make_body_var_ref(dv->getMention_c());
      break;
    case DefVar::DefVar_FORMAL:
      m_output = m_fact->make_arg_var_ref(dv->getMention_c());
      break;
    default:
      rcc_error("MakeVarRefVisitor: unrecognized DefVar::SourceT");
    }
  }

public:
  OA_ptr<R_VarRef> visit(Var * host) {
    host->accept(this);
    return m_output;
  }

private:
  OA_ptr<R_VarRef> m_output;
  VarRefFactory * m_fact;
};

LocalityDFSolver::LocalityDFSolver(OA_ptr<R_IRInterface> _rir)
  : m_ir(_rir)
  {}

/// Perform the data flow analysis. Sets each variable reference's Var
/// annotation with its locality information.
void LocalityDFSolver::
perform_analysis(ProcHandle proc, OA_ptr<CFG::CFGInterface> cfg) {
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;

  m_cfg = cfg;
  m_proc = proc;

  MakeVarRefVisitor visitor;

  // solve as a forward data flow problem
  m_solver = new DataFlow::CFGDFSolver(DataFlow::CFGDFSolver::Forward, *this);
  m_solver->solve(cfg, DataFlow::ITERATIVE);

  // data flow problem solved; now traverse the function and fill in annotations

  CFG_FOR_EACH_NODE(m_cfg, node) {
    OA_ptr<DFSet> in_set = m_solver->getInSet(node).convert<DFSet>();

    OA_ptr<CFG::NodeStatementsIteratorInterface> si = node->getNodeStatementsIterator();
    // statement-level CFG; there should be no more than one statement
    if (si->isValid()) {

      // each mention in the statement
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(si->current()));
      ExpressionInfo::var_iterator mi;
      for(mi = stmt_annot->begin_vars(); mi != stmt_annot->end_vars(); ++mi) {
	Var * annot = *mi;
	// get a VarRef
	OA_ptr<R_VarRef> ref = visitor.visit(annot);
	assert(!ref.ptrEqual(0));
	// look up the mention's name in in_set to get lattice type
	OA_ptr<DFSetElement> elem; elem = in_set->find(ref);
	if (elem.ptrEqual(NULL)) {
	  rcc_error("LocalityDFSolver: name of a mention not found in mNodeInSetMap");
	}
      
	// DF problem only analyzes uses and may-defs. Locality flags of
	// must-defs are already determined statically, so don't reset
	// them!
	if (annot->getMayMustType() == Var::Var_MAY
	    || annot->getUseDefType() == Var::Var_USE)
	{
	  annot->setScopeType(elem->get_locality_type());
	}
      } // next mention
      // ++*si; assert(!si->isValid());  // if >1 statement per node, something went wrong
      // TODO: add back assertion. Why does this sometimes fail?
    }
  }  // next CFG node
}

/// Print out a representation of the in and out sets for each CFG node.
void LocalityDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

/// Print out a representation of the in and out sets for each CFG node.
void LocalityDFSolver::dump_node_maps(ostream &os) {
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

//------------------------------------------------------------------
// Implementing the callbacks for CFGDFProblem
//------------------------------------------------------------------

/// Create a set of all variables set to TOP
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::initializeTop() {
  if (m_all_top.ptrEqual(NULL)) {
    initialize_sets();
  }
  return m_all_top->clone();
}

/// Create a set of all variables set to BOTTOM
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::initializeBottom() {
  if (m_all_bottom.ptrEqual(NULL)) {
    initialize_sets();
  }
  return m_all_bottom->clone();
}

void LocalityDFSolver::initialize_sets() {
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;

  m_all_top = new DFSet;
  m_all_bottom = new DFSet;
  m_entry_values = new DFSet;

  VarRefFactory * fact = VarRefFactory::get_instance();

  CFG_FOR_EACH_NODE(m_cfg, node) {
    NODE_FOR_EACH_STATEMENT(node, stmt) {
      if (debug) {
	Rf_PrintValue(make_sexp(stmt));
      }

      // getProperty will trigger lower-level analysis if necessary
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(stmt));

      // for this statement's annotation, iterate through its set of var mentions
      ExpressionInfo::const_var_iterator vi;
      for(vi = stmt_annot->begin_vars(); vi != stmt_annot->end_vars(); ++vi) {
	OA_ptr<R_VarRef> ref; ref = fact->make_body_var_ref((*vi)->getMention_c());

	// all_top and all_bottom: all variables set to TOP/BOTTOM,
	// must be initialized for OA data flow analysis
	OA_ptr<DFSetElement> top; top = new DFSetElement(ref, Locality_TOP);
	m_all_top->insert(top);
	OA_ptr<DFSetElement> bottom; bottom = new DFSetElement(ref, Locality_BOTTOM);
	m_all_bottom->insert(bottom);

	// entry_values: initial in set for entry node.  Formal
	// arguments are LOCAL; all other variables are FREE.  We add
	// all variables as FREE here, then reset the formals as LOCAL
	// after the per-node and per-statement stuff.
	OA_ptr<DFSetElement> free; free = new DFSetElement(ref, Locality_FREE);
	m_entry_values->insert(free);
      }
    } // statements
  } // CFG nodes
  
  // set all formals LOCAL instead of FREE on entry
  SEXP arglist = CAR(fundef_args_c(make_sexp(m_proc)));
  OA_ptr<R_VarRefSet> vs = R_VarRefSet::refs_from_arglist(arglist);
  m_all_top->insert_varset(vs, Locality_TOP);
  m_all_bottom->insert_varset(vs, Locality_BOTTOM);
  m_entry_values->insert_varset(vs, Locality_LOCAL);
}

/// The in set for the entry node gets the set of entry values (all
/// names FREE except formals, which are LOCAL). All other in sets are
/// initialized as TOP.
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::initializeNodeIN(OA_ptr<CFG::NodeInterface> n) {
  if (n.ptrEqual(m_cfg->getEntry())) {
    return m_entry_values->clone();
  } else {
    return m_all_top->clone();
  }
}

/// All out sets are initialized as TOP.
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::initializeNodeOUT(OA_ptr<CFG::NodeInterface> n) {
  return m_all_top->clone();
}

/// Meet function merges info from predecessors. CFGDFProblem says: OK
/// to modify set1 and return it as result, because solver only passes
/// a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::
meet(OA_ptr<DataFlow::DataFlowSet> set1, OA_ptr<DataFlow::DataFlowSet> set2) {
  OA_ptr<DFSet> retval;
  retval = meet_use_set(set1.convert<DFSet>(), set2->clone().convert<DFSet>());
  return retval.convert<DataFlow::DataFlowSet>();
}

/// Transfer function adds data given a statement. CFGDFProblem says:
/// OK to modify in set and return it again as result because solver
/// clones the BB in sets
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::
transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs, StmtHandle stmt_handle) {
  OA_ptr<DFSet> in; in = in_dfs.convert<DFSet>();
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(stmt_handle));
  ExpressionInfo::const_var_iterator var_iter;
  for(var_iter = annot->begin_vars(); var_iter != annot->end_vars(); ++var_iter) {
    // if variable was found to be local during statement-level
    // analysis, add it in
    if ((*var_iter)->getScopeType() == Locality_LOCAL) {
      MakeVarRefVisitor visitor;
      OA_ptr<R_VarRef> var; var = visitor.visit(*var_iter);
      OA_ptr<DFSetElement> use; use = new DFSetElement(var, (*var_iter)->getScopeType());
      in->replace(use);
    }
  }
  return in.convert<DataFlow::DataFlowSet>();
}

//--------------------------------------------------------------------
// Static meet functions
//--------------------------------------------------------------------

/// meet function for a single lattice element
LocalityType var_meet(LocalityType x, LocalityType y) {
  if (x == y) {
    return x;
  } else if (x == Locality_TOP) {
    return y;
  } else if (y == Locality_TOP) {
    return x;
  } else {
    return Locality_BOTTOM;
  }
}

/// Meet function for two DFSets, using the single-variable meet
/// operation when a use appears in both sets
OA_ptr<DFSet> meet_use_set(OA_ptr<DFSet> set1, OA_ptr<DFSet> set2) {
  // return value begins as set 1
  OA_ptr<DFSet> retval; retval = set1->clone().convert<DFSet>();
  OA_ptr<DFSetElement> use1;
  // check each element of set 2
  for(OA_ptr<DFSetIterator> it2 = set2->get_iterator(); it2->isValid(); ++*it2) {
    // if not in set 1 also, add it to the final set. If in both sets, meet the two elements.
    use1 = set1->find(it2->current()->get_loc());
    if (use1.ptrEqual(NULL)) {          // in set 2 only
      retval->insert(it2->current());
    } else {                            // in both sets
      LocalityType new_type = var_meet(use1->get_locality_type(), it2->current()->get_locality_type());
      retval->replace(use1->get_loc(), new_type);
    }
  }
  return retval;
}

}  // end namespace Locality
