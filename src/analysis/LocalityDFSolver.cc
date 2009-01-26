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

#include <support/Debug.h>
#include <support/RccError.h>

#include "LocalityDFSolver.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

namespace Locality {

// forward declarations

static LocalityType var_meet(LocalityType x, LocalityType y);
static OA_ptr<DFSet> meet_use_set(OA_ptr<DFSet> set1, OA_ptr<DFSet> set2);
static OA_ptr<R_VarRef> var_ref_from_use(UseVar * use);
static OA_ptr<R_VarRef> var_ref_from_def(DefVar * def);
static void initialize_set_element(OA_ptr<DFSet> set, Locality::LocalityType locality, OA_ptr<R_VarRef> ref);

// static variable for debugging

static bool debug;

OA_ptr<R_VarRef> var_ref_from_use(UseVar * use) {
  return VarRefFactory::get_instance()->make_body_var_ref(use->getMention_c());
}
  
OA_ptr<R_VarRef> var_ref_from_def(DefVar * def) {
  switch(def->getSourceType()) {
  case DefVar::DefVar_ASSIGN:
    return VarRefFactory::get_instance()->make_body_var_ref(def->getMention_c());
    break;
  case DefVar::DefVar_FORMAL:
    return VarRefFactory::get_instance()->make_arg_var_ref(def->getMention_c());
    break;
  default:
    rcc_error("MakeVarRefVisitor: unrecognized DefVar::SourceT");
  }
}


/// visitor that returns an R_VarRef of the appropriate type when
/// applied to Var annotation
class MakeVarRefVisitor : private VarVisitor {
public:
  explicit MakeVarRefVisitor() : m_output() {}
      
private:
  void visitUseVar(UseVar * use) {
    m_output = var_ref_from_use(use);
  }
  
  void visitDefVar(DefVar * def) {
    m_output = var_ref_from_def(def);
  }

public:
  OA_ptr<R_VarRef> visit(Var * host) {
    host->accept(this);
    return m_output;
  }

private:
  OA_ptr<R_VarRef> m_output;
};

LocalityDFSolver::LocalityDFSolver(OA_ptr<R_IRInterface> _rir)
  : m_ir(_rir)
{
  RCC_DEBUG("RCC_LocalityDFSolver", debug);
}

/// Perform the data flow analysis. Sets each variable reference's Var
/// annotation with its locality information.
void LocalityDFSolver::
perform_analysis(ProcHandle proc, OA_ptr<CFG::CFGInterface> cfg) {
  UseVar * use;
  DefVar * def;
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

      // DF problem only analyzes uses and may-defs. Locality flags of
      // must-defs are already determined statically, so don't reset
      // them!
      
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(si->current()));
      // set locality flag for all uses
      EXPRESSION_FOR_EACH_USE(stmt_annot, use) {
	OA_ptr<DFSetElement> elem; elem = look_up_var_ref(in_set, var_ref_from_use(use));
	use->setScopeType(elem->get_locality_type());
      }
      // set locality flag for may-defs
      EXPRESSION_FOR_EACH_DEF(stmt_annot, def) {
	OA_ptr<DFSetElement> elem; elem = look_up_var_ref(in_set, var_ref_from_def(def));
	if (def->getMayMustType() == Var::Var_MAY) {
	  def->setScopeType(elem->get_locality_type());
	}
      }
      // ++*si; assert(!si->isValid());  // if >1 statement per node, something went wrong
      // TODO: add back assertion. Why does this sometimes fail?
    }
  }  // next CFG node
}

OA_ptr<DFSetElement> LocalityDFSolver::look_up_var_ref(OA_ptr<DFSet> set, OA_ptr<R_VarRef> ref) {
  assert(!ref.ptrEqual(0));
  // look up the mention's name in in_set to get lattice type
  OA_ptr<DFSetElement> elem; elem = set->find(ref);
  if (elem.ptrEqual(NULL)) {
    rcc_error("LocalityDFSolver: name of a mention not found in mNodeInSetMap");
  }
  return elem;
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
  UseVar * use;
  DefVar * def;
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

      EXPRESSION_FOR_EACH_USE(stmt_annot, use) {
	OA_ptr<R_VarRef> ref; ref = var_ref_from_use(use);

	initialize_set_element(m_all_top, Locality_TOP, ref);
	initialize_set_element(m_all_bottom, Locality_BOTTOM, ref);
	initialize_set_element(m_entry_values, Locality_FREE, ref);
      }
      EXPRESSION_FOR_EACH_DEF(stmt_annot, def) {
	OA_ptr<R_VarRef> ref; ref = var_ref_from_def(def);

	initialize_set_element(m_all_top, Locality_TOP, ref);
	initialize_set_element(m_all_bottom, Locality_BOTTOM, ref);
	initialize_set_element(m_entry_values, Locality_FREE, ref);
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

void initialize_set_element(OA_ptr<DFSet> set, Locality::LocalityType locality, OA_ptr<R_VarRef> ref) {
  OA_ptr<DFSetElement> element; element = new DFSetElement(ref, locality);
  set->insert(element);
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
  UseVar * use;
  DefVar * def;
  OA_ptr<DFSet> in; in = in_dfs.convert<DFSet>();
  ExpressionInfo * annot = getProperty(ExpressionInfo, make_sexp(stmt_handle));
  // if variable was found to be local during statement-level
  // analysis, add it in
  EXPRESSION_FOR_EACH_USE(annot, use) {
    if (use->getScopeType() == Locality_LOCAL) {
      OA_ptr<R_VarRef> ref; ref = var_ref_from_use(use);
      OA_ptr<DFSetElement> elem; elem = new DFSetElement(ref, use->getScopeType());
      in->replace(elem);
    }
  }
  EXPRESSION_FOR_EACH_DEF(annot, def) { 
    if (def->getScopeType() == Locality_LOCAL) {
      OA_ptr<R_VarRef> ref; ref = var_ref_from_def(def);
      OA_ptr<DFSetElement> elem; elem = new DFSetElement(ref, def->getScopeType());
      in->replace(elem);
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
