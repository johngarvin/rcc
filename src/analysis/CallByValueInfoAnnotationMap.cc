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

// File: CallByValueInfoAnnotationMap.cc
//
// Maps each call site to CallByValueInfo with eager/lazy info for actual arguments.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <set>

#include <support/Debug.h>
#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/CallByValueInfo.h>
#include <analysis/ExpressionSideEffect.h>
#include <analysis/ExpressionSideEffectAnnotationMap.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/FormalArgInfoAnnotationMap.h>
#include <analysis/FuncInfo.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/OACallGraphAnnotation.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/PreDebutSideEffect.h>
#include <analysis/ResolvedArgsAnnotationMap.h>
#include <analysis/ResolvedArgs.h>
#include <analysis/Settings.h>
#include <analysis/SimpleIterators.h>
#include <analysis/SymbolTableFacade.h>
#include <analysis/Utils.h>

#include "CallByValueInfoAnnotationMap.h"

namespace RAnnot {

// ----- static function -----

bool is_cbv_safe(FormalArgInfo * formal, SEXP actual_c);

static bool debug;

CallByValueInfoAnnotationMap::CallByValueInfoAnnotationMap() {
  RCC_DEBUG("RCC_CallByValue", debug);
}

CallByValueInfoAnnotationMap::~CallByValueInfoAnnotationMap() {
  delete_map_values();
}

CallByValueInfoAnnotationMap * CallByValueInfoAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT CallByValueInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

CallByValueInfoAnnotationMap * CallByValueInfoAnnotationMap::s_instance = 0;
PropertyHndlT CallByValueInfoAnnotationMap::s_handle = "CallByValueInfo";

void CallByValueInfoAnnotationMap::create() {
  s_instance = new CallByValueInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

void CallByValueInfoAnnotationMap::compute() {
  int i;
  FuncInfo * fi;
  SymbolTableFacade * symbol_table = SymbolTableFacade::instance();

  // set of library procedures that cannot be called by value for any
  // reason. For example, 'library' calls deparsing functions on its
  // arguments.
  std::set<SEXP> unsafe_libs;
  unsafe_libs.insert(Rf_install("library"));
  unsafe_libs.insert(Rf_install("summary"));
  
  // analyze call sites (those that can be analyzed) and figure out
  // whether each arg can be made CBV
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_CALL_SITE(fi, csi_c) {
      SEXP cs = CAR(*csi_c);
      if (debug) {
	std::cout << "Analyzing call site:" << std::endl;
	Rf_PrintValue(cs);
	std::cout << std::endl;
      }
      
      CallByValueInfo * annot = new CallByValueInfo(Rf_length(call_args(cs)));
      get_map()[*csi_c] = annot;
     
      // if the callee is a library that's not user-redefined...
      if (is_var(call_lhs(cs)) && 
	  is_library(call_lhs(cs)) &&
	  getProperty(OACallGraphAnnotation, cs) == 0)
      {
	// for each arg, safe to make eager if it's trivial
	int i = 0;
	for(R_CallArgsIterator argi(cs); argi.isValid(); argi++, i++) {
	  if (unsafe_libs.find(call_lhs(cs)) != unsafe_libs.end()) {
	    annot->set_eager_lazy(i, LAZY);
	  } else {
	    if (getProperty(ExpressionSideEffect, argi.current())->is_trivial()) {
	      annot->set_eager_lazy(i, EAGER);
	    } else {
	      annot->set_eager_lazy(i, LAZY);
	    }
	  }
	}
	// TODO: add case to check if callee is strict or side effect free
      } else {  // user-defined, not a library

	// if unique callee can be found at compile time, grab it
	FuncInfo * callee;
	if (is_fundef(call_lhs(cs))) {
	  callee = getProperty(FuncInfo, CAR(call_lhs(cs)));
	} else if (is_var(call_lhs(cs))) {
	  VarInfo * vi = symbol_table->find_entry(cs); // not call_lhs; we want the cons cell
	  const DefVar * def = vi->single_def_if_exists();
	  if (def == 0) {
	    // procedures with no more than one user definition or undefined: no analysis, be conservative
	    rcc_warn("No CBV information for procedure with zero or more than one definition");
	    continue;
	  }
	  if (def->get_source_type() == DefVar::DefVar_ASSIGN && is_fundef(CAR(def->get_rhs_c()))) {
	    callee = getProperty(FuncInfo, CAR(def->get_rhs_c()));
	  } else {
	    rcc_warn("No CBV information for callee name defined as a non-fundef");
	    continue;
	  }
	} else {
	  rcc_warn("No CBV information for non-symbol LHS of procedure call");
	  continue;
	}
	
	if (callee->get_has_var_args()) continue; // next call site
	
	if (Settings::instance()->get_resolve_arguments()) {
	  ResolvedArgs * args_annot = getProperty(ResolvedArgs, *csi_c);

	  // set eager/lazy for each resolved arg
	  i = 0;
	  for(ResolvedArgs::const_iterator it = args_annot->begin(); it != args_annot->end(); it++) {
	    FormalArgInfo * formal = getProperty(FormalArgInfo, it->formal);
	    args_annot->set_eager_lazy(i, is_cbv_safe(formal, it->cell) ? EAGER : LAZY);
	    i++;
	  }

	  // set eager/lazy for each actual arg
	  i = 0;
	  for(R_CallArgsIterator argi(cs); argi.isValid(); argi++, i++) {
	    SEXP actual_c = argi.current();
	    FormalArgInfo * formal = getProperty(FormalArgInfo, args_annot->get_formal_for_actual(actual_c));
	    annot->set_eager_lazy(i, is_cbv_safe(formal, actual_c) ? EAGER : LAZY);
	  }
	} else {
	  // no argument resolution
	  // TODO: this assumes all args are in order with no defaults.
	  // set eager/lazy for each actual arg
	  i = 0;
	  for(R_CallArgsIterator argi(cs); argi.isValid(); argi++, i++) {
	    FormalArgInfo * formal = getProperty(FormalArgInfo, callee->get_arg(i+1));
	    SEXP actual_c = argi.current();
	    annot->set_eager_lazy(i, is_cbv_safe(formal, actual_c) ? EAGER : LAZY);
	  }
	}
      }
    }
  }
}

bool is_cbv_safe(FormalArgInfo * formal, SEXP actual_c) {
  // get side effect of the pre-debut part of the callee
  PreDebutSideEffect * pre_debut = getProperty(PreDebutSideEffect, formal->get_sexp());
  // formal->get_pre_debut_side_effect();

  if (TYPEOF(CAR(actual_c)) == DOTSXP) {
    return false;
  }
  
  if (CAR(actual_c) == R_MissingArg) {
    return true;
  }

  // get the actual argument's side effect
  ExpressionSideEffect * arg_side_effect = getProperty(ExpressionSideEffect, actual_c);

  if (debug) {
    std::cout << "Pre-debut side effect: ";
    pre_debut->dump(std::cout);
    std::cout << "Actual arg: ";
    Rf_PrintValue(CAR(actual_c));
    std::cout << "with side effect: ";
    arg_side_effect->dump(std::cout);
    std::cout << std::endl;
  }
  
  if (arg_side_effect->is_trivial() /* &&
      ( ! ParseInfo::get_command_line_args()->get_require_cheapness() ||
      arg_side_effect->is_cheap())) */ )
  {
    if (debug) {
      std::cout << "Actual is trivial" << std::endl;
    }
    
    return true;
  } else {
    if (formal->is_strict()) {

      if (debug) {
	std::cout << "Formal is strict:";
	formal->dump(std::cout);
	std::cout << std::endl;
	std::cout << "Testing dependence between actual arg: ";
	Rf_PrintValue(CAR(actual_c));
	std::cout << "and pre-debut code: ";
      }
      
      // test for intersection
      if (arg_side_effect->intersects(pre_debut->get_side_effect())) {
	if (debug) {
	  std::cout << "DEPENDENCE" << std::endl;
	}
	return false;
      } else {
	if (debug) {
	  std::cout << "NO DEPENDENCE" << std::endl;
	}
	return true;
      }
    } else {    // formal is non-strict
      if (debug) {
	std::cout << "Formal is non-trivial, non-strict: ";
	formal->dump(std::cout);
      }
      // CBV is safe if arg is trivial
      return false;
    }
  }
}

} // end namespace RAnnot
