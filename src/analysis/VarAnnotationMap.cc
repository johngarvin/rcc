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
{}
  
VarAnnotationMap::~VarAnnotationMap()
{}


// ----- singleton pattern -----

VarAnnotationMap * VarAnnotationMap::get_instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT VarAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

void VarAnnotationMap::create() {
  s_instance = new VarAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

VarAnnotationMap * VarAnnotationMap::s_instance = 0;
PropertyHndlT VarAnnotationMap::s_handle = "Var";


// ----- computation -----

// compute all Var annotation information
void VarAnnotationMap::compute() {
  FuncInfo * fi;
  FOR_EACH_PROC(fi) {
    compute_proc(fi);
  }
}
  
void VarAnnotationMap::compute_proc(FuncInfo * fi) {
  compute_proc_syntactic_info(fi);
  compute_proc_locality_info(fi);
}

// Compute syntactic variable info for the whole program. Refers to
// the ExpressionInfo annotation for each statement.
void VarAnnotationMap::compute_proc_syntactic_info(FuncInfo * fi) {
  UseVar * use;
  DefVar * def;
  OA_ptr<OA::CFG::NodeInterface> node;
  StmtHandle stmt;

  PROC_FOR_EACH_NODE(fi, node) {
    NODE_FOR_EACH_STATEMENT(node, stmt) {
      ExpressionInfo * expr = getProperty(ExpressionInfo, make_sexp(stmt));
      EXPRESSION_FOR_EACH_USE(expr, use) {
	assert(use != 0);
	get_map()[use->getMention_c()] = use;
      }
      EXPRESSION_FOR_EACH_DEF(expr, def) {
	assert(def != 0);
	get_map()[def->getMention_c()] = def;
      }
      
    }
  }
}

/// compute variable locality (bound/free) for each function
void VarAnnotationMap::compute_proc_locality_info(FuncInfo * fi) {
  R_Analyst * an = R_Analyst::get_instance();
  OA_ptr<R_IRInterface> interface; interface = an->get_interface();
  ProcHandle ph = make_proc_h(fi->get_sexp());
  OA_ptr<MyCFG> cfg = fi->get_cfg();
  compute_locality_info(interface, ph, cfg);
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


} // end namespace RAnnot
