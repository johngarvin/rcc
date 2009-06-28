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

// File: SideEffectAnnotationMap.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "SideEffectAnnotationMap.h"

#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/Alias/AliasTagSet.hpp>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/OACallGraphAnnotationMap.h>
#include <analysis/SideEffect.h>
#include <analysis/SimpleIterators.h>

#include <support/Debug.h>
#include <support/RccError.h>

using namespace OA;
using namespace HandleInterface;

static bool debug;

namespace RAnnot {

//  ----- constructor/destructor ----- 
  
SideEffectAnnotationMap::SideEffectAnnotationMap()
{
  RCC_DEBUG("RCC_SideEffect", debug);
}
  
SideEffectAnnotationMap::~SideEffectAnnotationMap()
{}

// ----- singleton pattern -----

SideEffectAnnotationMap * SideEffectAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT SideEffectAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void SideEffectAnnotationMap::create() {
  m_instance = new SideEffectAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

SideEffectAnnotationMap * SideEffectAnnotationMap::m_instance = 0;
PropertyHndlT SideEffectAnnotationMap::m_handle = "SideEffect";

// ----- computation -----

// compute all Var annotation information
void SideEffectAnnotationMap::compute() {
  init_non_action_libs();

  FuncInfo * fi;
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;

  compute_oa_side_effect();
  // now use m_side_effect to get info on expressions

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
      for (R_ListIterator arg_it(call_args(CAR(*csi))); arg_it.isValid(); ++arg_it) {
	if (debug) {
	  std::cout << "making side effect for actual arg: ";
	  Rf_PrintValue(CAR(arg_it.current()));
	}
	make_side_effect(fi, arg_it.current());
      }
    }
  }  // next function
}

void SideEffectAnnotationMap::compute_oa_side_effect() {
  // populate m_side_effect with OA side effect info

  OA_ptr<R_IRInterface> interface; interface = R_Analyst::get_instance()->get_interface();
  OA_ptr<CallGraph::CallGraphInterface> call_graph;
  call_graph = OACallGraphAnnotationMap::get_instance()->get_OA_call_graph();
  m_alias = OACallGraphAnnotationMap::get_instance()->get_OA_alias();

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

void SideEffectAnnotationMap::add_all_names_used(SideEffect * annot,
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

void SideEffectAnnotationMap::add_all_names_defined(SideEffect * annot,
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

void SideEffectAnnotationMap::make_side_effect(const FuncInfo * const fi, const SEXP cell) {
  SEXP cs_c;
  UseVar * use;
  DefVar * def;
  OA_ptr<Alias::AliasTagIterator> tag_iter;

  OA_ptr<Alias::Interface> alias = m_alias->getAliasResults(make_proc_h(fi->get_sexp()));
  SEXP e = CAR(cell);
  ExpressionInfo * expr = getProperty(ExpressionInfo, cell);
  SideEffect * annot = new SideEffect();

  annot->set_trivial(expression_is_trivial(e));
  annot->set_cheap(expression_is_cheap(e));

  // first grab local uses and defs
  EXPRESSION_FOR_EACH_USE(expr, use) {
    annot->insert_use_var(fi, use);
  }
  EXPRESSION_FOR_EACH_DEF(expr, def) {
    annot->insert_def_var(fi, def);
  }

  // now grab side effects due to procedure calls: get interprocedural
  // uses and defs from m_side_effect
  EXPRESSION_FOR_EACH_CALL_SITE(expr, cs_c) {
    if (expression_may_have_action(CAR(cs_c))) {
      annot->set_action(true);
    }
    
    CallHandle call_handle = make_call_h(CAR(cs_c));
    add_all_names_used(annot, alias, m_side_effect->getREFIterator(call_handle));
    add_all_names_defined(annot, alias, m_side_effect->getMODIterator(call_handle));
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
bool SideEffectAnnotationMap::expression_is_trivial(const SEXP e) {
  if (is_const(e) || is_var(e) || is_subscript(e)) {
    return true;
  } else if (is_paren_exp(e)) {
    return expression_is_trivial(CAR(paren_body_c(e)));
  } else {
    return false;
  }
}

// "cheaply evaluable" expressions take a "reasonably short" amount of
// time to evaluate. This is used for the call-by-value
// transformation: the transformation may be unprofitable if the
// callee is nonstrict in the formal argument and the corresponding
// actual argument is expensive to evaluate.
bool SideEffectAnnotationMap::expression_is_cheap(const SEXP e) {
  return (is_const(e) || is_var(e));
}

// true if the expression may perform a visible action, such as
// printing something to the screen.
bool SideEffectAnnotationMap::expression_may_have_action(const SEXP e) {
  if (is_var(call_lhs(e)) && is_library(call_lhs(e))) {
    // conservatively say true if not in the "safe" set
    return (m_non_action_libs.find(var_name(call_lhs(e))) == m_non_action_libs.end());
  } else {
    return false;
  }
}

// add library procedures that don't produce actions
void SideEffectAnnotationMap::init_non_action_libs() {
  m_non_action_libs.insert("+");
  m_non_action_libs.insert("-");
  m_non_action_libs.insert("*");
  m_non_action_libs.insert("/");
  m_non_action_libs.insert("%*%");
  m_non_action_libs.insert("c");
  m_non_action_libs.insert("rep");
  m_non_action_libs.insert("length");
  m_non_action_libs.insert("sum");
  m_non_action_libs.insert("vector");
  m_non_action_libs.insert("matrix");
  m_non_action_libs.insert("(");
  m_non_action_libs.insert(":");
  m_non_action_libs.insert("[");
  m_non_action_libs.insert("sqrt");
  m_non_action_libs.insert("mean");
  m_non_action_libs.insert("t");
  m_non_action_libs.insert("log");
  m_non_action_libs.insert("append");
}

} // end namespace RAnnot
