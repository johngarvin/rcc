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

static const bool debug = false;

CallByValueAnalysis::CallByValueAnalysis() {
}

void CallByValueAnalysis::perform_analysis() {
  SymbolTableFacade * symbol_table = SymbolTableFacade::get_instance();

  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_CALL_SITE(fi, csi) {
      if (debug) {
	std::cout << "Call site:" << std::endl;
	Rf_PrintValue(*csi);
      }

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
      if (debug) {
	cout << "Pre-debut side effect: ";
	pre_debut->dump(cout);
      }

      if (callee->get_num_args() != Rf_length(call_args(*csi))) {
	// TODO: handle default args and "..."
	throw AnalysisException();
      }
      // for each arg
      int i = 1;
      for(R_ListIterator argi(call_args(*csi)); argi.isValid(); argi++, i++) {
	FormalArgInfo * formal = getProperty(FormalArgInfo, callee->get_arg(i));

	// if not strict, conservatively call it CBN
	if (!formal->is_strict()) {
	  formal->set_is_value(false);
	  if (debug) {
	    cout << "nonstrict formal arg ";
	    formal->dump(std::cout);
	  }
	  continue;
	}

	// get the actual argument's side effect
	SEXP actual = argi.current();
	SideEffect * arg_side_effect = getProperty(SideEffect, actual);
	if (debug) {
	  cout << "Actual arg: ";
	  Rf_PrintValue(CAR(actual));
	  cout << "Arg side effect: ";
	  arg_side_effect->dump(cout);
	}

	if (arg_side_effect->intersects(pre_debut)) {
	  formal->set_is_value(false);
	  if (debug) {
	    std::cout << "dependence between actual arg ";
	    Rf_PrintValue(actual);
	    std::cout << "and pre-debut" << std::endl;
	  }
	  continue;
	}
	formal->set_is_value(true);
	if (debug) {
	  cout << "strict formal arg ";
	  formal->dump(std::cout);
	}
      }
    }
  }
}

SideEffect * CallByValueAnalysis::get_pre_debut_side_effect(FuncInfo * callee) {
  assert(callee != 0);
  SideEffect * pre_debut = new SideEffect();
  StrictnessDFSolver * strictness_solver; strictness_solver = new StrictnessDFSolver(R_Analyst::get_instance()->get_interface());
  OA_ptr<StrictnessResult> strictness;
  strictness = strictness_solver->perform_analysis(make_proc_h(callee->get_sexp()), callee->get_cfg());
  if (debug) strictness->dump(cout);

  PROC_FOR_EACH_NODE(callee, node) {
    // each statement in basic block
    OA_ptr<OA::CFG::NodeStatementsIteratorInterface> si = node->getNodeStatementsIterator();
    for(StmtHandle stmt; si->isValid(); ++*si) {
      stmt = si->current();
      ExpressionInfo * expr = getProperty(ExpressionInfo, make_sexp(stmt));
      // For this formal arg, statements in the function can be
      // divided into pre-debut, post-debut, and non-strict. We're
      // only interested in pre-debut statements. Since we're already
      // excluding non-strict functions (functions with non-strict
      // statements), we can get this set by excluding post-debut
      // statements.
      if (strictness->get_post_debut_stmts()->find(expr->get_sexp()) != strictness->get_post_debut_stmts()->end()) {
	continue;
      }
      pre_debut->add(getProperty(SideEffect, make_sexp(stmt)));
    }
  }
  return pre_debut;
}
