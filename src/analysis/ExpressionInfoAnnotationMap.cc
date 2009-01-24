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

// File: ExpressionInfoAnnotationMap.cc
//
// Maps expression SEXPs to ExpressionInfo annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <map>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

#include <support/Debug.h>
#include <support/RccError.h>

#include <analysis/Analyst.h>
#include <analysis/AnalysisResults.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/LocalVariableAnalysis.h>
#include <analysis/PropertySet.h>

#include "ExpressionInfoAnnotationMap.h"

static bool debug;

using namespace HandleInterface;

namespace RAnnot {

// type definitions for readability
typedef ExpressionInfoAnnotationMap::MyKeyT MyKeyT;
typedef ExpressionInfoAnnotationMap::MyMappedT MyMappedT;
typedef ExpressionInfoAnnotationMap::iterator iterator;
typedef ExpressionInfoAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor ----- 

ExpressionInfoAnnotationMap::ExpressionInfoAnnotationMap()
{
  RCC_DEBUG("RCC_ExpressionInfoAnnotationMap", debug);
}

ExpressionInfoAnnotationMap::~ExpressionInfoAnnotationMap() {
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = get_map().begin(); iter != get_map().end(); ++iter) {
    delete(iter->second);
  }
}

// ----- computation -----

void ExpressionInfoAnnotationMap::compute() {
  FuncInfo * fi;
  OA::OA_ptr<OA::CFG::NodeInterface> node;
  OA::StmtHandle stmt;

  // we need to make expressions out of statements, call sites, and actual arguments
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_NODE(fi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	// statements
	ExpressionInfo * annot = make_annot(make_sexp(stmt));
	for(ExpressionInfo::const_call_site_iterator csi = annot->begin_call_sites(); csi != annot->end_call_sites(); ++csi) {
	  // call sites
	  make_annot(*csi);
	  for(R_ListIterator arg_it(CAR(*csi)); arg_it.isValid(); ++arg_it) {
	    // actual arguments
	    make_annot(arg_it.current());
	  }
	}
      }
    }
  }
}

ExpressionInfo * ExpressionInfoAnnotationMap::make_annot(const MyKeyT & k) {
  if (debug) {
    std::cout << "ExpressionInfoAnnotationMap analyzing the following expression:" << std::endl;
    Rf_PrintValue(CAR(k));
  }

  ExpressionInfo * annot = new ExpressionInfo(k);
  LocalVariableAnalysis lva(k);
  lva.perform_analysis();
  LocalVariableAnalysis::const_var_iterator v;
  for(v = lva.begin_vars(); v != lva.end_vars(); ++v) {
    assert(*v != 0);
    annot->insert_var(*v);
  }
  LocalVariableAnalysis::const_call_site_iterator cs;
  for(cs = lva.begin_call_sites(); cs != lva.end_call_sites(); ++cs) {
    annot->insert_call_site(*cs);
  }

  get_map()[k] = annot;
  return annot;
}

// ----- singleton pattern -----

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT ExpressionInfoAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void ExpressionInfoAnnotationMap::create() {
  m_instance = new ExpressionInfoAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::m_instance = 0;
PropertyHndlT ExpressionInfoAnnotationMap::m_handle = "ExpressionInfo";

}
