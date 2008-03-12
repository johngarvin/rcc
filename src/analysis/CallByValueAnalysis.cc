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
#include <analysis/SimpleIterators.h>
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

CallByValueAnalysis::CallByValueAnalysis() {
}

void CallByValueAnalysis::perform_analysis() {
  SymbolTableFacade * symbol_table = SymbolTableFacade::get_instance();

  // for each call site of each function
  FuncInfoIterator fii(R_Analyst::get_instance()->get_scope_tree_root());
  for(FuncInfo *fi; fii.IsValid(); fii++) {
    fi = fii.Current();
    FuncInfo::const_call_site_iterator csi;
    for(csi = fi->begin_call_sites(); csi != fi->end_call_sites(); ++csi) {
      // get side effect of the pre-debut part of the callee
      FuncInfo * callee;
      if (is_fundef(call_lhs(*csi))) {
	callee = getProperty(FuncInfo, CAR(call_lhs(*csi)));
      } else if (is_var(call_lhs(*csi))) {
	VarInfo * vi = symbol_table->find_entry(fi, getProperty(Var, *csi)); // not call_lhs; Var wants the cons cell
	DefVar * def = vi->single_def_if_exists();
	if (def == 0) {
	  continue;
	  // TODO: what should be done here? Need to handle library procedures with side effects
	  // and procedures with more than one definition.
	}
	if (is_fundef(CAR(def->getRhs_c()))) {
	  callee = getProperty(FuncInfo, CAR(def->getRhs_c()));
	} else {
	  continue;
	}
      } else {
	throw AnalysisException();
      }
      SideEffect * pre_debut = get_pre_debut_side_effect(callee);

      // for each arg
      int i = 1;
      for(R_ListIterator argi(call_args(*csi)); argi.isValid(); argi++, i++) {
	FormalArgInfo * formal = getProperty(FormalArgInfo, callee->get_arg(i));
	if (TAG(argi.current()) == Rf_install("...")) {
	  // we are not yet handling varargs
	  throw AnalysisException();
	}

	// if not strict, conservatively call it CBN
	if (!formal->is_strict()) {
	  formal->set_is_value(false);
	  continue;
	}

	// get the actual argument's side effect
	SEXP actual = argi.current();
	SideEffect * arg_side_effect = getProperty(SideEffect, actual);

	if (arg_side_effect->intersects(pre_debut)) {
	  formal->set_is_value(false);
	  continue;
	}
	formal->set_is_value(true);
      }
    }
  }
}

SideEffect * CallByValueAnalysis::get_pre_debut_side_effect(FuncInfo * callee) {
  assert(callee != 0);
  SideEffect * pre_debut = new SideEffect();
  StrictnessDFSolver * strictness_solver; strictness_solver = new StrictnessDFSolver(R_Analyst::get_instance()->get_interface());
  OA_ptr<StrictnessResult> strictness;
  strictness = strictness_solver->perform_analysis(make_proc_h(callee->get_defn()), callee->get_cfg());

  OA_ptr<OA::CFG::NodesIteratorInterface> ni = callee->get_cfg()->getCFGNodesIterator();
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
      pre_debut->add(getProperty(SideEffect, make_sexp(stmt)));
    }
  }
  return pre_debut;
}
