// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: SideEffectAnnotationMap.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "SideEffectAnnotationMap.h"

#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/Alias/ManagerInterAliasMapBasic.hpp>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/VarSet.h>

using namespace OA;
using namespace HandleInterface;

namespace RAnnot {

//  ----- constructor/destructor ----- 
  
SideEffectAnnotationMap::SideEffectAnnotationMap()
{}
  
SideEffectAnnotationMap::~SideEffectAnnotationMap()
{}

// ----- singleton pattern -----

SideEffectAnnotationMap * SideEffectAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT SideEffectAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void SideEffectAnnotationMap::create() {
  m_instance = new SideEffectAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

SideEffectAnnotationMap * SideEffectAnnotationMap::m_instance = 0;
PropertyHndlT SideEffectAnnotationMap::m_handle = "SideEffect";

// ----- computation -----

// compute all Var annotation information
void SideEffectAnnotationMap::compute() {
  // for each function
  FuncInfoIterator fii(R_Analyst::get_instance()->get_scope_tree_root());
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
	VarSet * annot = dynamic_cast<VarSet *>(get_map()[expr->getDefn()]);
	// each variable in the expression
	ExpressionInfo::const_var_iterator vi;
	for(vi = expr->begin_vars(); vi != expr->end_vars(); ++vi) {
	  if ((*vi)->getUseDefType() == Var::Var_USE) {
	    annot->insert_use(*vi);
	  } else if ((*vi)->getUseDefType() == Var::Var_DEF) {
	    annot->insert_def(*vi);
	  }
	}
	ExpressionInfo::const_call_site_iterator csi;
	for(csi = expr->begin_call_sites(); csi != expr->end_call_sites(); ++csi) {
	  
	  // TODO: add MOD/REF info to map
	  // m_call_graph->getMODIterator()
	  // m_call_graph->getREFIterator()
	}
      }
    }
  }
}

} // end namespace RAnnot
