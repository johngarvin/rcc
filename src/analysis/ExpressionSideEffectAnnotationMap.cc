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

// File: ExpressionSideEffectAnnotationMap.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "ExpressionSideEffectAnnotationMap.h"

#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/ResolvedArgs.h>
#include <analysis/ResolvedArgsAnnotationMap.h>
#include <analysis/Settings.h>
#include <analysis/SideEffect.h>
#include <analysis/SimpleIterators.h>

#include <support/Debug.h>
#include <support/RccError.h>

using namespace OA;
using namespace HandleInterface;

static bool debug;

namespace RAnnot {

// ----- SideEffectLibMap methods

SideEffectLibMap::SideEffectLibMap() : m_map()
{
}

void SideEffectLibMap::insert(std::string key, bool side_effects, bool expensive, bool error) {
  std::vector<bool> value(3);
  value[0] = side_effects;
  value[1] = expensive;
  value[2] = error;
  m_map[key] = value;
}

bool SideEffectLibMap::get(std::string key, int which) {
  if (m_map.find(key) == m_map.end()) {
    return true;
  } else {
    return m_map[key][which];
  }
}


//  ----- constructor/destructor ----- 
  
ExpressionSideEffectAnnotationMap::ExpressionSideEffectAnnotationMap()
{
  RCC_DEBUG("RCC_ExpressionSideEffect", debug);
}
  
ExpressionSideEffectAnnotationMap::~ExpressionSideEffectAnnotationMap()
{}

// ----- singleton pattern -----

ExpressionSideEffectAnnotationMap * ExpressionSideEffectAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT ExpressionSideEffectAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

void ExpressionSideEffectAnnotationMap::create() {
  s_instance = new ExpressionSideEffectAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

ExpressionSideEffectAnnotationMap * ExpressionSideEffectAnnotationMap::s_instance = 0;
PropertyHndlT ExpressionSideEffectAnnotationMap::s_handle = "ExpressionSideEffect";

// ----- computation -----

// compute all Var annotation information
void ExpressionSideEffectAnnotationMap::compute() {
  init_lib_data();

  FuncInfo * fi;
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;

  compute_oa_side_effect();
  // now use m_side_effect to get info on expressions

  // get side effects for default arguments
  FOR_EACH_PROC(fi) {
    for (SEXP arg = fi->get_args(); arg != R_NilValue; arg = CDR(arg)) {
      if (CAR(arg) != R_MissingArg) {
	make_side_effect(fi, arg);
      }
    }
  }

  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_NODE(fi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	if (debug) {
	  std::cout << "making side effect for statement: ";
	  Rf_PrintValue(CAR(make_sexp(stmt)));
	}
	make_side_effect(fi, make_sexp(stmt));
      }  // next statement
    }  // next node

    PROC_FOR_EACH_CALL_SITE(fi, csi) {
      // left side
      make_side_effect(fi, CAR(*csi));

      // actual args
      for (R_CallArgsIterator arg_it(CAR(*csi)); arg_it.isValid(); ++arg_it) {
	if (debug) {
	  std::cout << "making side effect for actual arg: ";
	  Rf_PrintValue(CAR(arg_it.current()));
	}
	make_side_effect(fi, arg_it.current());
      }
    }
  }  // next function
}

/// populate m_side_effect with OA side effect info
void ExpressionSideEffectAnnotationMap::compute_oa_side_effect() {
  OA_ptr<R_IRInterface> interface; interface = R_Analyst::instance()->get_interface();
  OA_ptr<CallGraph::CallGraphInterface> call_graph;
  call_graph = OACallGraphAnnotationMap::instance()->get_OA_call_graph();
  m_alias = OACallGraphAnnotationMap::instance()->get_OA_alias();

  OA::SideEffect::ManagerInterSideEffectStandard solver(interface);
  // param bindings
  DataFlow::ManagerParamBindings pb_man(interface);
  OA_ptr<DataFlow::ParamBindings> param_bindings; param_bindings = pb_man.performAnalysis(call_graph);

  // intra side effect information
  OA_ptr<OA::SideEffect::ManagerSideEffectStandard> intra_man;
  intra_man = new OA::SideEffect::ManagerSideEffectStandard(interface);

  // compute side effect information
  m_side_effect = solver.performAnalysis(call_graph, param_bindings, m_alias, intra_man, DataFlow::ITERATIVE);
}

void ExpressionSideEffectAnnotationMap::add_all_names_used(SideEffect * annot,
						 OA_ptr<Alias::Interface> alias,
						 OA_ptr<Alias::AliasTagIterator> tag_iter)
{
  for(; tag_iter->isValid(); ++(*tag_iter)) {
    OA_ptr<MemRefExprIterator> mem_iter = alias->getMemRefExprIterator(tag_iter->current());
    for (; mem_iter->isValid(); ++(*mem_iter)) {
      if (mem_iter->current()->isaNamed()) {
	OA_ptr<NamedRef> ref = mem_iter->current().convert<NamedRef>();
	annot->insert_use(make_var_info(ref->getSymHandle()));
      } else {
	throw new AnalysisException("Not yet implemented");
      }
    }
  }
}

void ExpressionSideEffectAnnotationMap::add_all_names_defined(SideEffect * annot,
						    OA_ptr<Alias::Interface> alias,
						    OA_ptr<Alias::AliasTagIterator> tag_iter)
{
  for(; tag_iter->isValid(); ++(*tag_iter)) {
    OA_ptr<MemRefExprIterator> mem_iter = alias->getMemRefExprIterator(tag_iter->current());
    for (; mem_iter->isValid(); ++(*mem_iter)) {
      if (mem_iter->current()->isaNamed()) {
	OA_ptr<NamedRef> ref = mem_iter->current().convert<NamedRef>();
	annot->insert_def(make_var_info(ref->getSymHandle()));
      } else {
	throw new AnalysisException("Not yet implemented");
      }
    }
  }
}

void ExpressionSideEffectAnnotationMap::make_side_effect(const FuncInfo * const fi, const SEXP cell) {
  SEXP cs_c, use, def;
  OA_ptr<Alias::AliasTagIterator> tag_iter;

  OA_ptr<Alias::Interface> alias = m_alias->getAliasResults(make_proc_h(fi->get_sexp()));
  SEXP e = CAR(cell);
  ExpressionInfo * expr = getProperty(ExpressionInfo, cell);
  ExpressionSideEffect * annot = new ExpressionSideEffect(expression_is_trivial(e), expression_is_cheap(e));

  // first grab local uses and defs
  EXPRESSION_FOR_EACH_USE(expr, use) {
    annot->insert_use_sexp(fi, use);
  }
  EXPRESSION_FOR_EACH_DEF(expr, def) {
    annot->insert_def_sexp(fi, def);
  }

  // now grab side effects due to procedure calls: get interprocedural
  // uses and defs from m_side_effect
  EXPRESSION_FOR_EACH_CALL_SITE(expr, cs_c) {
    if (call_may_have_action(CAR(cs_c))) {
      annot->set_action(true);
    }
    
    CallHandle call_handle = make_call_h(CAR(cs_c));
    add_all_names_used(annot->get_side_effect(), alias, m_side_effect->getREFIterator(call_handle));
    add_all_names_defined(annot->get_side_effect(), alias, m_side_effect->getMODIterator(call_handle));
  }
  
  if (debug) {
    std::cout << "Side effect produced: ";
    annot->dump(std::cout);
  }
  get_map()[cell] = annot;
} 

// an expression is trivially evaluable if it cannot diverge, throw an
// error/exception, or have any side effect. Used for call-by-value
// transformation: if the callee is non-strict in some formal
// argument, then the call-by-value transformation is valid if there
// is no dependence between pre-debut code and the corresponding
// actual argument and the actual argument is trivially evaluable.
bool ExpressionSideEffectAnnotationMap::expression_is_trivial(const SEXP e) {
  if (is_const(e) || is_var(e) || is_subscript(e)) {
    return true;

    // TODO: and (assume-correct-program or call cannot throw error)
  } else if (is_call(e) &&
	     ! call_may_have_action(e) &&
	     ! call_may_throw_error(e))
    {
      for(SEXP x = call_args(e); x != R_NilValue; x = CDR(x)) {
	if (! expression_is_trivial(CAR(x))) {
	  return false;
	}
      }
      return true;
    } else {
    return false;
  }
}

    
  
// "cheaply evaluable" expressions take a "reasonably short" amount of
// time to evaluate. This is used for the call-by-value
// transformation: the transformation may be unprofitable if the
// callee is nonstrict in the formal argument and the corresponding
// actual argument is expensive to evaluate.
bool ExpressionSideEffectAnnotationMap::expression_is_cheap(const SEXP e) {
  if (is_const(e) || is_var(e)) {
    return true;
  } else if (is_call(e) && ! call_may_be_expensive(e)) {
    for(SEXP x = call_args(e); x != R_NilValue; x = CDR(x)) {
      if (!expression_is_cheap(CAR(x))) {
	return false;
      }
    }
    return true;
  } else {
    return false;
  }
}

bool is_library_call(const SEXP e) {
  return (is_var(call_lhs(e)) && is_library(call_lhs(e)));
}
// true if the library call may perform a visible action, such as
// 'print' printing something to the screen.
bool ExpressionSideEffectAnnotationMap::call_may_have_action(const SEXP e) {
  if (is_library_call(e)) {
    return (m_non_action_libs.get(var_name(call_lhs(e)), 0));
  } else {
    return true;
  }
}

bool ExpressionSideEffectAnnotationMap::call_may_be_expensive(const SEXP e) {
  if (Settings::instance()->get_aggressive_cbv()) {
    return false;
  } else if (is_library_call(e)) {
    return m_non_action_libs.get(var_name(call_lhs(e)), 1);
  } else {
    return true;
  }
}

bool ExpressionSideEffectAnnotationMap::call_may_throw_error(const SEXP e) {
  if (Settings::instance()->get_assume_correct_program()) {
    return false;
  } else if (is_library_call(e)) {
    return m_non_action_libs.get(var_name(call_lhs(e)), 2);
  } else {
    return true;
  }
}

// init_lib_data: provide analysis data for library procedures
//
// If a library is not in the map, side_effects, expensive, and error
// are all considered true.
//
// side_effects: this procedure may have side effects
//   example: rnorm modifies the random number generator seed
// expensive: this procedure may be expensive (generally, more than O(1))
//   example: arithmetic if arguments are large arrays
// error: this procedure may throw an error or exception
//   example: relational operators given non-conformable arrays
void ExpressionSideEffectAnnotationMap::init_lib_data() {
  //                       lib          side_effects expensive error
  m_non_action_libs.insert("+",         false,       true,     false);
  m_non_action_libs.insert("-",         false,       true,     false);
  m_non_action_libs.insert("*",         false,       true,     false);
  m_non_action_libs.insert("/",         false,       true,     false);
  m_non_action_libs.insert("^",         false,       true,     false);
  m_non_action_libs.insert("%%",        false,       true,     false);
  m_non_action_libs.insert("%/%",       false,       true,     false);
  m_non_action_libs.insert("==",        false,       true,     true);
  m_non_action_libs.insert("!=",        false,       true,     true);
  m_non_action_libs.insert("<",         false,       true,     true);
  m_non_action_libs.insert(">",         false,       true,     true);
  m_non_action_libs.insert("<=",        false,       true,     true);
  m_non_action_libs.insert(">=",        false,       true,     true);
  m_non_action_libs.insert("%*%",       false,       true,     false);
  m_non_action_libs.insert("c",         false,       true,     false);
  m_non_action_libs.insert("rep",       false,       true,     false);
  m_non_action_libs.insert("length",    false,       true,     false);
  m_non_action_libs.insert("sum",       false,       true,     false);
  m_non_action_libs.insert("vector",    false,       false,    false);
  m_non_action_libs.insert("matrix",    false,       false,    false);
  m_non_action_libs.insert("(",         false,       false,    false);
  m_non_action_libs.insert(":",         false,       false,    false);
  m_non_action_libs.insert("[",         false,       false,    false);
  m_non_action_libs.insert("sqrt",      false,       true,     false);
  m_non_action_libs.insert("mean",      false,       true,     false);
  m_non_action_libs.insert("t",         false,       true,     false);
  m_non_action_libs.insert("log",       false,       true,     false);
  m_non_action_libs.insert("append",    false,       true,     false);
  m_non_action_libs.insert("$",         false,       false,    false);
}

} // end namespace RAnnot
