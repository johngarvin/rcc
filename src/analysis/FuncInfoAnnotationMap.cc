// -*- Mode: C++ -*-
//
// Copyright (c) 2010 Rice University
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

// File: FuncInfoAnnotationMap.cc
//
// Set of FuncInfo annotations representing function information,
// including the methods for computing the information. This should
// probably be split up into the different analyses at some point.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <list>

#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/Assertion.h>
#include <analysis/BasicFuncInfoAnnotationMap.h>
#include <analysis/ExpressionInfoAnnotationMap.h>
#include <analysis/HandleInterface.h>
#include <analysis/Utils.h>
#include <analysis/VarAnnotationMap.h>

#include "FuncInfoAnnotationMap.h"

using namespace HandleInterface;

namespace RAnnot {
  
// type definitions for readability
typedef FuncInfoAnnotationMap::MyKeyT MyKeyT;
typedef FuncInfoAnnotationMap::MyMappedT MyMappedT;
typedef FuncInfoAnnotationMap::iterator iterator;
typedef FuncInfoAnnotationMap::const_iterator const_iterator;

//  ----- constructor/destructor ----- 

FuncInfoAnnotationMap::FuncInfoAnnotationMap() {}

FuncInfoAnnotationMap::~FuncInfoAnnotationMap() {
  // This AnnotationMap owns all FuncInfos, so we want to delete them now.
  // FuncInfos are in JMC's tree structure; deleting the root will delete all the FuncInfos.
  delete m_root;
}

// ----- singleton pattern -----

FuncInfoAnnotationMap * FuncInfoAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT FuncInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

void FuncInfoAnnotationMap::create() {
  s_instance = new FuncInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

FuncInfo * FuncInfoAnnotationMap::get_scope_tree_root() {
  if (s_instance == 0) {
    create();
  }
  compute_if_necessary();
  return m_root;
}

FuncInfoAnnotationMap * FuncInfoAnnotationMap::s_instance = 0;
PropertyHndlT FuncInfoAnnotationMap::s_handle = "FuncInfo";

// ----- computation -----

void FuncInfoAnnotationMap::compute() {
  BasicFuncInfo * basic = BasicFuncInfoAnnotationMap::instance()->get_scope_tree_root();
  build_tree(0, basic);
  m_root = static_cast<FuncInfo *>(get_map()[basic->get_sexp()]);
}

void FuncInfoAnnotationMap::build_tree(FuncInfo * parent, BasicFuncInfo * basic) {
  FuncInfo * fi = new FuncInfo(parent, basic);
  get_map()[basic->get_sexp()] = fi;
  NonUniformDegreeTreeNodeChildIteratorTmpl<BasicFuncInfo> child_it(basic);
  for (child_it.Reset(); child_it.IsValid(); ++child_it) {
    build_tree(fi, child_it.Current());
  }
}

void FuncInfoAnnotationMap::collect_libraries() {
  FuncInfo * fi;
  std::list<SEXP> libs;
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_CALL_SITE(fi, csi) {
      SEXP lhs = call_lhs(CAR(*csi));
      if (is_var(lhs) && is_library(lhs) && is_closure(library_value(lhs))) {
	BasicFuncInfo * new_bfi; new_bfi = new BasicFuncInfo(0, library_value(lhs), library_value(lhs));
	new_bfi->perform_analysis();
	putProperty(BasicFuncInfo, library_value(lhs), new_bfi);
	FuncInfo * new_fi; new_fi = new FuncInfo(0, new_bfi);
	get_map()[library_value(lhs)] = new_fi;
	libs.push_back(library_value(lhs));
      }
    }
  }
  for(std::list<SEXP>::const_iterator iter = libs.begin(); iter != libs.end(); ++iter) {
    FuncInfo * fi = getProperty(FuncInfo, *iter);
    OA::OA_ptr<OA::CFG::NodeInterface> node;
    OA::StmtHandle stmt;
    PROC_FOR_EACH_NODE(fi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	ExpressionInfoAnnotationMap::instance()->make_annot(make_sexp(stmt));
      }
    }
    VarAnnotationMap::instance()->compute_proc(fi->get_basic());
  }
}

}
