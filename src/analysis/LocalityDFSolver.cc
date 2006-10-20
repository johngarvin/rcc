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

// File: LocalityDFSolver.h
//
// Implements the OpenAnalysis CFG data flow problem interface for a
// function to determine whether variable references refer to local or
// free variables.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/Interface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/AnalysisResults.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/Var.h>
#include <analysis/UseVar.h>
#include <analysis/DefVar.h>

#include <analysis/LocalityDFSet.h>
#include <analysis/LocalityDFSetElement.h>
#include <analysis/LocalityDFSetIterator.h>

#include <support/RccError.h>

#include "LocalityDFSolver.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

// forward declarations

static LocalityType var_meet(LocalityType x, LocalityType y);
static OA_ptr<Locality::DFSet> meet_use_set(OA_ptr<Locality::DFSet> set1, OA_ptr<Locality::DFSet> set2);
static R_VarRef * make_var_ref_from_annotation(Var * annot);

// static variable for debugging

static const bool debug = false;

namespace Locality {

/// Initialize as a forward data flow problem.
LocalityDFSolver::LocalityDFSolver(OA_ptr<R_IRInterface> _rir)
  : DataFlow::CFGDFProblem( DataFlow::Forward ), rir(_rir)
  {}

/// Perform the data flow analysis. Sets each variable reference's Var
/// annotation with its locality information.
void LocalityDFSolver::
perform_analysis(ProcHandle proc, OA_ptr<CFG::Interface> cfg) {
  m_cfg = cfg;
  m_proc = proc;

  // Note: ReachConsts explicitly uses base class name:
  //  DataFlow::CFGDFProblem::solve(cfg);
  // Why?
  solve(cfg);
  // solve populates mNodeInSetMap and mNodeOutSetMap

  // For reference:
  //std::map<OA_ptr<CFG::Interface::Node>,OA_ptr<DataFlowSet> > mNodeInSetMap;
  //std::map<OA_ptr<CFG::Interface::Node>,OA_ptr<DataFlowSet> > mNodeOutSetMap;

  // each node
  std::map<OA_ptr<CFG::Interface::Node>,OA_ptr<DataFlow::DataFlowSet> >::iterator mi;
  for(mi = mNodeInSetMap.begin(); mi != mNodeInSetMap.end(); ++mi) {

    // map contains ptrs to base class DataFlowSet; convert to ptr to derived class DFSet
    OA_ptr<DFSet> in_set = mi->second.convert<DFSet>();

    OA_ptr<CFG::Interface::NodeStatementsIterator> si;
    si = mi->first->getNodeStatementsIterator();
    // statement-level CFG; there should be no more than one statement
    if (si->isValid()) {

      // each mention in the statement
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(si->current()));
      ExpressionInfo::var_iterator mi;
      for(mi = stmt_annot->begin_vars(); mi != stmt_annot->end_vars(); ++mi) {
	Var * annot = *mi;
	// look up the mention's name in in_set to get lattice type
	OA_ptr<R_VarRef> ref; ref = make_var_ref_from_annotation(annot);
	OA_ptr<DFSetElement> elem; elem = in_set->find(ref);
	if (elem.ptrEqual(NULL)) {
	  rcc_error("Name of a mention not found in mNodeInSetMap");
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
      // FIXME: add back assertion. Why does this sometimes fail?
    }
  }  // next CFG node
}

void LocalityDFSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

/// Print out a representation of the in and out sets for each CFG node.
void LocalityDFSolver::dump_node_maps(ostream &os) {
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
    in_set->dump(os, rir);
    os << "OUT SET:\n";
    out_set->dump(os, rir);
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

/// Create a set of all variable set to BOTTOM
OA_ptr<DataFlow::DataFlowSet> LocalityDFSolver::initializeBottom() {
  if (m_all_bottom.ptrEqual(NULL)) {
    initialize_sets();
  }
  return m_all_bottom->clone();
}

void LocalityDFSolver::initialize_sets() {
  m_all_top = new DFSet;
  m_all_bottom = new DFSet;
  m_entry_values = new DFSet;

  // each CFG node
  OA_ptr<CFG::Interface::NodesIterator> ni;
  ni = m_cfg->getNodesIterator();
  for ( ; ni->isValid(); ++*ni) {

    // each statement
    OA_ptr<CFG::Interface::NodeStatementsIterator> si; si = ni->current()->getNodeStatementsIterator();
    for( ; si->isValid(); ++*si) {
      if (debug) {
	Rf_PrintValue(make_sexp(si->current()));
      }

      // getProperty will trigger lower-level analysis if necessary
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(si->current()));

      // for this statement's annotation, iterate through its set of var mentions
      ExpressionInfo::const_var_iterator vi;
      for(vi = stmt_annot->begin_vars(); vi != stmt_annot->end_vars(); ++vi) {
	OA_ptr<R_VarRef> ref; ref = new R_BodyVarRef((*vi)->getMention_c());

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

/// Creates in and out DFSets and stores them in mNodeInSetMap and
/// mNodeOutSetMap.
void LocalityDFSolver::initializeNode(OA_ptr<CFG::Interface::Node> n) {
  if (n.ptrEqual(m_cfg->getEntry())) {
    mNodeInSetMap[n] = m_entry_values->clone();
  } else {
    mNodeInSetMap[n] = m_all_top->clone();
  }
  mNodeOutSetMap[n] = m_all_top->clone();
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
      OA_ptr<R_VarRef> var; var = make_var_ref_from_annotation(*var_iter);
      OA_ptr<DFSetElement> use; use = new DFSetElement(var, (*var_iter)->getScopeType());
      in->replace(use);
    }
  }
  return in.convert<DataFlow::DataFlowSet>();
}

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
OA_ptr<Locality::DFSet> meet_use_set(OA_ptr<Locality::DFSet> set1, OA_ptr<Locality::DFSet> set2) {
  // return value begins as set 1
  OA_ptr<Locality::DFSet> retval; retval = set1->clone().convert<Locality::DFSet>();
  OA_ptr<Locality::DFSetElement> use1;
  // check each element of set 2
  for(OA_ptr<Locality::DFSetIterator> it2 = set2->get_iterator(); it2->isValid(); ++*it2) {
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

//------------------------------------------------------------
// Other static functions
//------------------------------------------------------------

/// Creates and returns an R_VarRef element from the information
/// contained in a Var annotation. The caller is responsible for
/// managing the memory created. (Sticking it in an OA_ptr will do.)
/// TODO: make polymorphic to remove awkward switch on derived type
static R_VarRef * make_var_ref_from_annotation(RAnnot::Var * annot) {
  if (dynamic_cast<UseVar *>(annot)) {
    return new R_BodyVarRef(annot->getMention_c());
  } else if (DefVar * def_annot = dynamic_cast<DefVar *>(annot)) {
    switch(def_annot->getSourceType()) {
    case DefVar::DefVar_ASSIGN:
      return new R_BodyVarRef(annot->getMention_c());
      break;
    case DefVar::DefVar_FORMAL:
      return new R_ArgVarRef(annot->getMention_c());
      break;
    default:
      rcc_error("make_var_ref_from_annotation: unrecognized DefVar::SourceT");
    }
  } else {
    rcc_error("make_var_ref_from_annotation: unknown annotation type");
  }
  return 0;
}


