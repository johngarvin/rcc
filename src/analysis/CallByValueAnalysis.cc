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

#include <support/Debug.h>
#include <support/RccError.h>

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

static bool debug;

CallByValueAnalysis::CallByValueAnalysis() {
  RCC_DEBUG("RCC_CallByValue", debug);
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
    PROC_FOR_EACH_CALL_SITE(fi, csi_c) {
      SEXP cs = CAR(*csi_c);
      if (debug) {
	std::cout << "Analyzing call site:" << std::endl;
	Rf_PrintValue(cs);
	std::cout << std::endl;
      }
      
      ExpressionInfo * call_expr = getProperty(ExpressionInfo, *csi_c);
      
      // if unique callee can be found at compile time, grab it
      FuncInfo * callee;
      if (is_fundef(call_lhs(cs))) {
	callee = getProperty(FuncInfo, CAR(call_lhs(cs)));
      } else if (is_var(call_lhs(cs))) {
	VarInfo * vi = symbol_table->find_entry(fi, getProperty(Var, cs)); // not call_lhs; Var wants the cons cell
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
	throw AnalysisException("CallByValueAnalysis: non-symbol LHS of procedure call");
      }

      if (callee->get_num_args() != Rf_length(call_args(cs))) {
	// TODO: handle default args and "..."
	throw AnalysisException("CallByValueAnalysis: default args or \"...\"");
      }

      // for each arg
      int i = 1;
      for(R_ListIterator argi(call_args(cs)); argi.isValid(); argi++, i++) {
	FormalArgInfo * formal = getProperty(FormalArgInfo, callee->get_arg(i));
	SEXP actual = argi.current();
	call_expr->set_eager_lazy(i, is_cbv_safe(formal, actual) ? EAGER : LAZY);
      }
    }
  }
}

bool CallByValueAnalysis::is_cbv_safe(FormalArgInfo * formal, SEXP actual) {
  // get side effect of the pre-debut part of the callee
  SideEffect * pre_debut = formal->get_pre_debut_side_effect();
  
  if (debug) {
    cout << "Pre-debut side effect: ";
    pre_debut->dump(cout);
  }
  
  // if formal is not strict, conservatively say it's not safe
  if (!formal->is_strict()) {
    if (debug) {
      cout << "nonstrict formal arg: ";
      formal->dump(std::cout);
      cout << std::endl;
    }
    return false;
  }
  
  // get the actual argument's side effect
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
    if (debug) {
      std::cout << "dependence between actual arg: ";
      Rf_PrintValue(CAR(actual));
      std::cout << "and pre-debut code" << std::endl;
    }
    return false;
  }

  // if we get here, good news: no dependence, so we can use eager eval
  if (debug) {
    cout << "strict formal arg: ";
    formal->dump(std::cout);
  }
  return true;
}

SideEffect * CallByValueAnalysis::compute_pre_debut_side_effect(FuncInfo * fi, FormalArgInfo * fai) {
  assert(fi != 0);
  assert(fai != 0);

  SideEffect * pre_debut = new SideEffect();
  OA_ptr<OA::CFG::NodeInterface> node;
  StmtHandle stmt;

  if (TAG(fai->get_sexp()) == R_DotsSymbol) {
    return pre_debut;                          // if we have a "..." argument, give it an empty side effect
  }

  PROC_FOR_EACH_NODE(fi, node) {
    NODE_FOR_EACH_STATEMENT(node, stmt) {
      ExpressionInfo * expr = getProperty(ExpressionInfo, make_sexp(stmt));
      // For this formal arg, statements in the function can be
      // divided into pre-debut, post-debut, and non-strict. We're
      // only interested in pre-debut statements. Since we're already
      // excluding non-strict functions (functions with non-strict
      // statements), we can get this set by excluding post-debut
      // statements.
      OA_ptr<StrictnessResult> strictness = fi->get_strictness();
      OA_ptr<NameStmtMultiMap> post_debuts = strictness->get_post_debut_stmts();

      // TODO: refactor this. Maybe post_debuts should be a vector of sets, not a multimap
      pair<NameStmtMultiMap::const_iterator, NameStmtMultiMap::const_iterator> range = post_debuts->equal_range(TAG(fai->get_sexp()));
      bool stmt_is_post_debut = false;
      // TODO: we need to differentiate between missing formal
      // (compiler error) and formal with no post-debut side effect
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
      pre_debut->add(getProperty(SideEffect, make_sexp(stmt)));
      if (debug) {
	std::cout << "Found pre-debut statement:" << std::endl;
	Rf_PrintValue(CAR(make_sexp(stmt)));
      }
    }
  }
  return pre_debut;
}
