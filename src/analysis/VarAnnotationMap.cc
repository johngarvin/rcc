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

// File: VarAnnotationMap.cc
//
// Set of Var annotations, representing basic variable information,
// including the methods for computing the information. Maps a cons
// cell containing a SYMSXP to a Var annotation.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/CFG/CFG.hpp>

#include <support/RccError.h>

#include <analysis/Analyst.h>
#include <analysis/AnalysisResults.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/LocalityDFSolver.h>
#include <analysis/PropertySet.h>
#include <analysis/Var.h>

#include "VarAnnotationMap.h"

using namespace OA;
using namespace HandleInterface;

typedef CFG::CFGInterface MyCFG;

namespace RAnnot {
  
// ----- type definitions for readability -----

typedef VarAnnotationMap::MyKeyT MyKeyT;
typedef VarAnnotationMap::MyMappedT MyMappedT;
typedef VarAnnotationMap::iterator iterator;
typedef VarAnnotationMap::const_iterator const_iterator;

//  ----- constructor/destructor ----- 
  
VarAnnotationMap::VarAnnotationMap()
  : m_computed(false),
    m_map()
  {}
  
VarAnnotationMap::~VarAnnotationMap() {
  map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = m_map.begin(); iter != m_map.end(); ++iter) {
    delete(iter->second);
  }
}

// ----- singleton pattern -----

VarAnnotationMap * VarAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT VarAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void VarAnnotationMap::create() {
  m_instance = new VarAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

VarAnnotationMap * VarAnnotationMap::m_instance = 0;
PropertyHndlT VarAnnotationMap::m_handle = "Var";

//  ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// TODO: delete this when fully refactored to disallow insertion from outside.
MyMappedT & VarAnnotationMap::operator[](const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }

  return m_map[k];
}

// Perform the computation if necessary and returns the requested
// data.
MyMappedT VarAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    rcc_error("VarAnnotationMap: possible invalid key not found in map");
  }

  return annot->second;
}

bool VarAnnotationMap::is_valid(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  return (annot != m_map.end());
}
    
  
bool VarAnnotationMap::is_computed() const {
  return m_computed;
}

//  ----- iterators ----- 

iterator VarAnnotationMap::begin() { return m_map.begin(); }
iterator VarAnnotationMap::end() { return m_map.end(); }
const_iterator VarAnnotationMap::begin() const { return m_map.begin(); }
const_iterator VarAnnotationMap::end() const { return m_map.end(); }

// ----- computation -----

// compute all Var annotation information
void VarAnnotationMap::compute() {
  compute_all_syntactic_info();
  compute_all_locality_info();
}
  
// Compute syntactic variable info for the whole program. Refers to
// the ExpressionInfo annotation for each statement.
void VarAnnotationMap::compute_all_syntactic_info() {
  R_Analyst * an = R_Analyst::get_instance();
  // for each function
  FuncInfoIterator fii(an->get_scope_tree_root());
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    // for each CFG node (basic block)
    OA_ptr<CFG::NodesIteratorInterface> ni = fi->get_cfg()->getCFGNodesIterator();
    for (OA_ptr<CFG::Node> node; ni->isValid(); ++*ni) {
      node = ni->current().convert<CFG::Node>();
      // each statement in basic block
      OA_ptr<CFG::NodeStatementsIteratorInterface> si = node->getNodeStatementsIterator();
      for(StmtHandle stmt; si->isValid(); ++*si) {
	stmt = si->current();
	ExpressionInfo * expr = getProperty(ExpressionInfo, make_sexp(stmt));
	// each variable in the expression
	ExpressionInfo::const_var_iterator ei;
	for(ei = expr->begin_vars(); ei != expr->end_vars(); ++ei) {
	  // add Var annotation to our map
	  Var * v = *ei;
	  m_map[v->getMention_c()] = v;
	}
      }
    }
  }
}

/// compute variable locality (bound/free) for each function
void VarAnnotationMap::compute_all_locality_info() {
  R_Analyst * an = R_Analyst::get_instance();
  OA_ptr<R_IRInterface> interface; interface = an->get_interface();
  FuncInfo * root = an->get_scope_tree_root();
  FuncInfoIterator fii(root);
  for(FuncInfo * fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    ProcHandle ph = make_proc_h(fi->get_defn());
    OA_ptr<MyCFG> cfg = fi->get_cfg();
    compute_locality_info(interface, ph, cfg);
  }
}

/// For a given procedure, solve the locality data flow problem; put
/// the info in m_map.
void VarAnnotationMap::compute_locality_info(OA_ptr<R_IRInterface> interface,
					     ProcHandle proc,
					     OA_ptr<MyCFG> cfg)
{
  Locality::LocalityDFSolver solver(interface);
  solver.perform_analysis(proc, cfg);
}

}
