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

// File: PreDebutSideEffectAnnotationMap.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "PreDebutSideEffectAnnotationMap.h"

#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/ExpressionSideEffect.h>
#include <analysis/ExpressionSideEffectAnnotationMap.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/ResolvedArgs.h>
#include <analysis/ResolvedArgsAnnotationMap.h>
#include <analysis/Settings.h>
#include <analysis/SideEffect.h>
#include <analysis/SimpleIterators.h>
#include <analysis/StrictnessResult.h>

#include <support/Debug.h>
#include <support/RccError.h>

using namespace OA;
using namespace HandleInterface;

static bool debug;

namespace RAnnot {

//  ----- constructor/destructor ----- 
  
PreDebutSideEffectAnnotationMap::PreDebutSideEffectAnnotationMap()
{
  RCC_DEBUG("RCC_PreDebutSideEffect", debug);
}
  
PreDebutSideEffectAnnotationMap::~PreDebutSideEffectAnnotationMap()
{}

// ----- singleton pattern -----

PreDebutSideEffectAnnotationMap * PreDebutSideEffectAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT PreDebutSideEffectAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

void PreDebutSideEffectAnnotationMap::create() {
  s_instance = new PreDebutSideEffectAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

PreDebutSideEffectAnnotationMap * PreDebutSideEffectAnnotationMap::s_instance = 0;
PropertyHndlT PreDebutSideEffectAnnotationMap::s_handle = "PreDebutSideEffect";

// ----- computation -----

// compute all Var annotation information
void PreDebutSideEffectAnnotationMap::compute() {
  FuncInfo * fi;

  FOR_EACH_PROC(fi) {
    // formals are indexed by 1
    for(int i=1; i<=fi->get_num_args(); i++) {
      get_map()[fi->get_arg(i)] = compute_pre_debut_side_effect(fi, fi->get_arg(i));
    }
  }
}

PreDebutSideEffect * PreDebutSideEffectAnnotationMap::compute_pre_debut_side_effect(FuncInfo * fi, SEXP formal)
{
  assert(fi != 0);
  
  PreDebutSideEffect * pre_debut = new PreDebutSideEffect(false, false);
  OA_ptr<OA::CFG::NodeInterface> node;
  StmtHandle stmt;

  if (TAG(formal) == R_DotsSymbol) {
    return pre_debut;                          // if we have a "..." argument, give it an empty side effect
  }

  // For any formal arg, statements in the function can be
  // divided into pre-debut, post-debut, and non-strict. We're
  // only interested in pre-debut statements. Since we're already
  // excluding non-strict functions (functions with non-strict
  // statements), we can get this set by excluding post-debut
  // statements.
  OA_ptr<Strictness::StrictnessResult> strictness = fi->get_strictness();
  OA_ptr<NameStmtMultiMap> post_debuts = strictness->get_post_debut_stmts();
  
  PROC_FOR_EACH_NODE(fi, node) {
    NODE_FOR_EACH_STATEMENT(node, stmt) {
      ExpressionInfo * expr = getProperty(ExpressionInfo, make_sexp(stmt));
      // TODO: refactor this. Maybe post_debuts should be a vector of sets, not a multimap
      std::pair<NameStmtMultiMap::const_iterator, NameStmtMultiMap::const_iterator> range;
      range = post_debuts->equal_range(TAG(formal));
      bool stmt_is_post_debut = false;
      // TODO: we need to differentiate between missing formal
      // (compiler error) and formal with no post-debut side effect.
      // Then check for missing formal here with something like
      // if (range.first == range.second) {
      //   rcc_error(...);
      // }
      for(NameStmtMultiMap::const_iterator it = range.first; it != range.second; ++it) {
	if (it->second == stmt) {
	  stmt_is_post_debut = true;
	  continue;
	}
      }
      if (stmt_is_post_debut) {
	continue;
      }
      pre_debut->add(getProperty(ExpressionSideEffect, make_sexp(stmt))->get_side_effect());
      if (debug) {
	std::cout << "Found pre-debut statement:" << std::endl;
	Rf_PrintValue(CAR(make_sexp(stmt)));
      }
    }
  }
  return pre_debut;
  
}

} // end namespace RAnnot
