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
//
// File: BasicFuncInfoAnnotationMap.cc
//
// Set of FuncInfo annotations representing function information,
// including the methods for computing the information. This should
// probably be split up into the different analyses at some point.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/Assertion.h>
#include <analysis/HandleInterface.h>
#include <analysis/Utils.h>

#include "BasicFuncInfoAnnotationMap.h"

namespace RAnnot {

  // ----- constructor/deconstructor -----

BasicFuncInfoAnnotationMap::BasicFuncInfoAnnotationMap() {}

BasicFuncInfoAnnotationMap::~BasicFuncInfoAnnotationMap() {
  // This AnnotationMap owns all BasicFuncInfos, so we want to delete them now.
  // BasicFuncInfos are in JMC's tree structure; deleting the root will delete all the BasicFuncInfos.
  delete m_root;
}

// ----- singleton pattern -----

BasicFuncInfoAnnotationMap * BasicFuncInfoAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT BasicFuncInfoAnnotationMap::handle()
{
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

BasicFuncInfo * BasicFuncInfoAnnotationMap::get_scope_tree_root()
{
  if (s_instance == 0) {
    create();
  }
  compute_if_necessary();
  return m_root;
}

// ----- computation -----

void BasicFuncInfoAnnotationMap::compute() {
  SEXP r_root = R_Analyst::instance()->get_program();
  assert(is_simple_assign(r_root));

  build_scope_tree(r_root);
}

void BasicFuncInfoAnnotationMap::build_scope_tree(SEXP r_root) {
  // create the root FuncInfo node
  SEXP name_c = assign_lhs_c(r_root);
  SEXP definition = CAR(assign_rhs_c(r_root));
  assert(is_fundef(definition));
  m_root = new BasicFuncInfo(0, name_c, definition); // root node has null parent
  get_map()[definition] = m_root;
  
  // Skip to the body of the function. Otherwise, the definition we
  // just recorded would be flagged as a duplicate "anonymous"
  // function.
  build_scope_tree_rec(CAR(fundef_body_c(definition)), m_root);
}

void BasicFuncInfoAnnotationMap::build_scope_tree_rec(SEXP e, BasicFuncInfo * parent) {
  switch(TYPEOF(e)) {
  case NILSXP:
  case REALSXP:
  case STRSXP:
  case LGLSXP:
  case SYMSXP:
    return;
    break;
  case LISTSXP:
    build_scope_tree_rec(CAR(e), parent);
    build_scope_tree_rec(CDR(e), parent);
    break;
  case LANGSXP:
    if (is_simple_assign(e)) {
      SEXP name_c = assign_lhs_c(e);
      SEXP rhs = CAR(assign_rhs_c(e));
      if (is_fundef(rhs)) {                  // a variable bound to a function
	BasicFuncInfo * newfun = new BasicFuncInfo(parent, name_c, rhs);
	get_map()[rhs] = newfun;

	// Skip to the body of the function. Otherwise, the definition we
	// just recorded would be flagged as a duplicate "anonymous"
	// function.
	build_scope_tree_rec(CAR(fundef_body_c(rhs)), newfun);
      } else {
	build_scope_tree_rec(rhs, parent);
      }
    } else if (is_fundef(e)) {  // anonymous function
      BasicFuncInfo * newfun = new BasicFuncInfo(parent, Rf_cons(Rf_install("<unknown function>"), R_NilValue), e);
      get_map()[e] = newfun;
      build_scope_tree_rec(CAR(fundef_body_c(e)), newfun);
    } else if (is_rcc_assertion(e)) { // rcc_assert call
      // Assertions are processed here.
      BasicFuncInfo * fi;
      if (is_rcc_assert_exp(e)) {
	// rcc.assert.exp assertion: 
	SEXP body = CADR(e);
	build_scope_tree_rec(body, parent);
	if (is_simple_assign(body)) {
	  SEXP rhs = CAR(assign_rhs_c(body));
	  if (is_fundef(rhs)) fi = getProperty(BasicFuncInfo, rhs); // TODO: is this right?
	}
      } else if (is_rcc_assert(e)) {
	fi = parent;
      }
      process_assert(e, fi);
    } else {                   // ordinary function call
      build_scope_tree_rec(CAR(e), parent);
      build_scope_tree_rec(CDR(e), parent);
    }
    break;
  default:
    assert(0);
  }
}

BasicFuncInfoAnnotationMap * BasicFuncInfoAnnotationMap::s_instance = 0;
PropertyHndlT BasicFuncInfoAnnotationMap::s_handle = "BasicFuncInfo";

void BasicFuncInfoAnnotationMap::create() {
  s_instance = new BasicFuncInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}


} // namespace RAnnot
