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

#include <support/Debug.h>
#include <support/RccError.h>

#include <analysis/Analyst.h>
#include <analysis/AnalysisResults.h>
#include <analysis/BasicFuncInfo.h>
#include <analysis/BasicFuncInfoAnnotationMap.h>
#include <analysis/BasicVarAnnotationMap.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/LocalityDFSolver.h>
#include <analysis/PropertySet.h>
#include <analysis/Var.h>

#include "VarAnnotationMap.h"

using namespace OA;
using namespace HandleInterface;

typedef CFG::CFGInterface MyCFG;

static bool debug;

namespace RAnnot {
  
// ----- type definitions for readability -----

typedef VarAnnotationMap::MyKeyT MyKeyT;
typedef VarAnnotationMap::MyMappedT MyMappedT;
typedef VarAnnotationMap::iterator iterator;
typedef VarAnnotationMap::const_iterator const_iterator;

//  ----- constructor/destructor ----- 
  
VarAnnotationMap::VarAnnotationMap()
{
  RCC_DEBUG("RCC_VarAnnotationMap", debug);
}
  
VarAnnotationMap::~VarAnnotationMap()
{}


// ----- singleton pattern -----

VarAnnotationMap * VarAnnotationMap::instance() {
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
  BasicFuncInfo * fi;
  for (BasicVarAnnotationMap::const_iterator it = BasicVarAnnotationMap::instance()->begin(); it != BasicVarAnnotationMap::instance()->end(); ++it) {
    BasicVar * bv = dynamic_cast<BasicVar *>(it->second);
    Var * var = new Var(bv);
    if (debug) {
      std::cout << "VarAnnotationMap adding: ";
      var->dump(std::cout);
    }
    get_map()[bv->get_mention_c()] = var;
  }
  
  FOR_EACH_BASIC_PROC(fi) {
    compute_proc_locality_info(fi);
  }
}
  
/// compute variable locality (bound/free) for each function
void VarAnnotationMap::compute_proc_locality_info(BasicFuncInfo * fi) {
  R_Analyst * an = R_Analyst::instance();
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
  typedef map<SEXP, Locality::LocalityType> MyMap;

  Locality::LocalityDFSolver solver(interface);
  MyMap map = solver.perform_analysis(proc, cfg);
  for(MyMap::const_iterator it = map.begin(); it != map.end(); it++) {
    Var * var = dynamic_cast<Var *>(get_map().at(it->first));
    var->set_scope_type(it->second);
    if (debug) {
      Rf_PrintValue(CAR(var->get_mention_c()));
      std::cout << "set to " << type_name(it->second) << std::endl;
    }
  }
}


} // end namespace RAnnot
