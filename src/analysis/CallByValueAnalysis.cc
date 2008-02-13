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

// File: CallByValueAnalysis.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/CFG/CFGInterface.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/DefVar.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/ScopeAnnotationMap.h>
#include <analysis/SideEffect.h>
#include <analysis/SideEffectAnnotationMap.h>
#include <analysis/StrictnessDFSolver.h>
#include <analysis/StrictnessResult.h>
#include <analysis/SymbolTableFacade.h>
#include <analysis/VarInfo.h>

#include "CallByValueAnalysis.h"

using namespace HandleInterface;
using namespace RAnnot;
using namespace Strictness;
using OA::OA_ptr;
using OA::StmtHandle;

void CallByValueAnalysis::perform_analysis() {
  SymbolTableFacade * symbol_table = SymbolTableFacade::get_instance();

  // for each function
  FuncInfoIterator fii(R_Analyst::get_instance()->get_scope_tree_root());
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    FuncInfo::const_call_site_iterator csi;
    for(csi = fi->begin_call_sites(); csi != fi->end_call_sites(); ++csi) {
      for(int i = 1; i <= fi->get_num_args(); i++) {
	// the actual argument's side effect
	SideEffect * arg_side_effect = getProperty(SideEffect, fi->get_arg(i));

	// the side effect of the pre-debut part of the callee
	VarInfo * vi = symbol_table->find_entry(fi, getProperty(Var, call_lhs(*csi)));
	DefVar * def = vi->single_def_if_exists();
	if (def == 0) {
	  throw new AnalysisException();
	}
	ScopeAnnotationMap * scope_map = ScopeAnnotationMap::get_instance();
	FuncInfo * callee = dynamic_cast<FuncInfo*>((*scope_map)[def->getName()]);
	StrictnessDFSolver * strictness_solver; strictness_solver = new StrictnessDFSolver(R_Analyst::get_instance()->get_interface());
	OA_ptr<StrictnessResult> strictness;
	strictness = strictness_solver->perform_analysis(make_proc_h(callee->get_defn()), callee->get_cfg());
	FormalArgInfo * formal = getProperty(FormalArgInfo, callee->get_arg(i));
	// if not strict, conservatively call it CBN
	if (!formal->is_strict()) {
	  formal->set_is_value(false);
	  continue;
	}
	SideEffect * pre_debut = new SideEffect();  // pre-debut statements of the callee
	OA_ptr<OA::CFG::NodesIteratorInterface> ni = fi->get_cfg()->getCFGNodesIterator();
	for(OA_ptr<OA::CFG::Node> node; ni->isValid(); ++*ni) {
	  node = ni->current().convert<OA::CFG::Node>();
	  // each statement in basic block
	  OA_ptr<OA::CFG::NodeStatementsIteratorInterface> si = node->getNodeStatementsIterator();
	  for(StmtHandle stmt; si->isValid(); ++*si) {
	    stmt = si->current();
	    ExpressionInfo * expr = getProperty(ExpressionInfo, make_sexp(stmt));
	    // For this formal arg, statements in the function can be
	    // divided into pre-debut, post-debut, and non-strict.
	    // We're only interested in pre-debut statements. Since
	    // we're already excluding non-strict functions (functions
	    // with non-strict statements), we can approximate this by
	    // just excluding post-debut statements.
	    if (strictness->get_post_debut_stmts()->find(expr->getDefn()) != 0) {
	      continue;
	    }
	    // pre_debut->union(strictness->get_post_debut_stmts());
	  }
	}
	
	// if (arg_side_effect->get_defs()->intersect(pre_debut->get_uses()) is empty) {
	//   formal->set_is_value(false);
	//   continue;
	// }
	// if (arg_side_effect->get_uses()->intersect(pre_debut->get_defs()) is empty) {
	//   formal->set_is_value(false);
	//   continue;
	// }
	// if (arg_side_effect->get_defs()->intersect(pre_debut->get_defs()) is empty) {
	//   formal->set_is_value(false);
	//   continue;
	// }
	// formal->set_is_value(true);
      }
    }

      // for each actual arg
      //   get arg expression's side effect
      //   look up function name in symbol table to get FuncInfo of callee
      //   if callee is strict in corresponding formal, arg must remain CBN
      //   let pre_debut be a SideEffect (with uses and defs)
      //   for each expression in func body
      //     continue if reached by a debut (we're looking at pre-debut)
      //     add side effect to pre_debut
      //   compare arg def with pre_use
      //     if any members in common, must remain CBN
      //   compare arg use with pre_def
      //     if any members in common, must remain CBN
      //   compare arg def with arg def
      //     if any members in common, must remain CBN
      //   if none in common, arg may be CBV
  }
}
