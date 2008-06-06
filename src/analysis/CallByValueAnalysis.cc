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
#include <analysis/EagerLazy.h>
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

// for each procedure:
//   make StrictnessDFSolver and perform analysis for each procedure, get StrictnessResult
//   for each formal arg:
//     compute, store side effect of pre-debut statements for that formal in FormalArgInfo
// for each call site:
//   get unique callee if it exists
//   for each actual arg:
//     compute side effect of actual
//     get pre-debut side effect of callee for this formal (stored in FormalArgInfo)
//     if intersection is empty:
//       store the fact that the actual can be called CBV-wise
//     else if the intersection is nonempty (a dependence exists):
//       store the fact that we must use CBN

void CallByValueAnalysis::perform_analysis() {
  FuncInfo * fi;
  SymbolTableFacade * symbol_table = SymbolTableFacade::get_instance();

  FOR_EACH_PROC(fi) {
    StrictnessDFSolver * strictness_solver; strictness_solver = new StrictnessDFSolver(R_Analyst::get_instance()->get_interface());
    OA_ptr<StrictnessResult> strictness;
    strictness = strictness_solver->perform_analysis(make_proc_h(fi->get_sexp()), fi->get_cfg());
    if (debug) strictness->dump(cout);
    fi->set_strictness(strictness);
    
    // formals are indexed by 1
    for(int i=1; i<=fi->get_num_args(); i++) {
      FormalArgInfo * formal = getProperty(FormalArgInfo, fi->get_arg(i));
      formal->set_pre_debut_side_effect(compute_pre_debut_side_effect(fi, formal));
    }
  }
  
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_CALL_SITE(fi, csi) {
      if (debug) {
	std::cout << "Analyzing call site:" << std::endl;
	Rf_PrintValue(*csi);
	std::cout << std::endl;
      }

      ExpressionInfo * call_expr = getProperty(ExpressionInfo, *csi);

      // if unique callee can be found at compile time, grab it
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

      if (callee->get_num_args() != Rf_length(call_args(*csi))) {
	// TODO: handle default args and "..."
	throw AnalysisException();
      }

      // for each arg
      int i = 1;
      for(R_ListIterator argi(call_args(*csi)); argi.isValid(); argi++, i++) {
	FormalArgInfo * formal = getProperty(FormalArgInfo, callee->get_arg(i));

	// get side effect of the pre-debut part of the callee
	SideEffect * pre_debut = formal->get_pre_debut_side_effect();

	if (debug) {
	  cout << "Pre-debut side effect: ";
	  pre_debut->dump(cout);
	}

	// if formal is not strict, conservatively call it CBN
	if (!formal->is_strict()) {
	  if (debug) {
	    cout << "nonstrict formal arg ";
	    formal->dump(std::cout);
	    cout << std::endl;
	  }

	  call_expr->set_eager_lazy(i, LAZY);
	  continue;
	}

	// get the actual argument's side effect
	SEXP actual = argi.current();
	SideEffect * arg_side_effect = getProperty(SideEffect, actual);
	if (debug) {
	  cout << "Actual arg: ";
	  Rf_PrintValue(CAR(actual));
	  cout << "with side effect: ";
	  arg_side_effect->dump(cout);
	  cout << std::endl;
	}

	// test for intersection
	if (arg_side_effect->intersects(pre_debut)) {
	  call_expr->set_eager_lazy(i, LAZY);
	  if (debug) {
	    std::cout << "dependence between actual arg ";
	    Rf_PrintValue(actual);
	    std::cout << "and pre-debut" << std::endl;
	  }
	  continue;
	}
	// if we get here, good news: no dependence, so we can use eager eval
	call_expr->set_eager_lazy(i, EAGER);
	if (debug) {
	  cout << "strict formal arg ";
	  formal->dump(std::cout);
	}
      }
    }
  }
}

SideEffect * CallByValueAnalysis::compute_pre_debut_side_effect(FuncInfo * fi, FormalArgInfo * fai) {
  assert(fi != 0);
  assert(fai != 0);

  SideEffect * pre_debut = new SideEffect();
  OA_ptr<OA::CFG::NodeInterface> node;
  PROC_FOR_EACH_NODE(fi, node) {
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
      OA_ptr<StrictnessResult> strictness = fi->get_strictness();
      OA_ptr<NameStmtMultiMap> post_debuts = strictness->get_post_debut_stmts();
      if (post_debuts->find(TAG(fai->get_sexp())) != post_debuts->end()) {  // if the formal is in the post_debut set
	continue;
      }
      pre_debut->add(getProperty(SideEffect, make_sexp(stmt)));
    }
  }
  return pre_debut;
}
