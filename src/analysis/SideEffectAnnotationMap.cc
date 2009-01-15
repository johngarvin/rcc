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
#include <OpenAnalysis/Alias/ManagerInterAliasMapBasic.hpp>
#include <OpenAnalysis/Location/Location.hpp>
#include <OpenAnalysis/Location/NamedLoc.hpp>

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
      for(R_ListIterator arg_it(CAR(*csi)); arg_it.isValid(); ++arg_it) {
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
  OA_ptr<Alias::InterAliasInterface> alias;
  alias = OACallGraphAnnotationMap::get_instance()->get_OA_alias();

  OA::SideEffect::ManagerInterSideEffectStandard solver(interface);
  // param bindings
  DataFlow::ManagerParamBindings pb_man(interface);
  OA_ptr<DataFlow::ParamBindings> param_bindings; param_bindings = pb_man.performAnalysis(call_graph);

  // intra side effect information
  OA_ptr<OA::SideEffect::ManagerSideEffectStandard> intra_man;
  intra_man = new OA::SideEffect::ManagerSideEffectStandard(interface);

  // compute side effect information
  m_side_effect = solver.performAnalysis(call_graph, param_bindings, alias, intra_man, DataFlow::ITERATIVE);
}

void SideEffectAnnotationMap::make_side_effect(const FuncInfo * const fi, const SEXP cell) {
  SEXP cs_c;
  Var * var;

  SEXP e = CAR(cell);
  ExpressionInfo * expr = getProperty(ExpressionInfo, cell);
  SideEffect * annot = new SideEffect();

  annot->set_trivial(expression_is_trivial(e));
  annot->set_cheap(expression_is_cheap(e));

  // first grab local uses and defs

  EXPRESSION_FOR_EACH_MENTION(expr, var) {
    annot->insert_mention(fi, var);
  }

  // now grab interprocedural uses and defs from m_side_effect

  EXPRESSION_FOR_EACH_CALL_SITE(expr, cs_c) {
    OA_ptr<LocIterator> li;
    for(li = m_side_effect->getMODIterator(make_call_h(CAR(cs_c))); li->isValid(); ++(*li)) {
      OA_ptr<OA::Location> location; location = li->current();
      if (location->isaNamed()) {
	OA_ptr<NamedLoc> named_loc; named_loc = location.convert<NamedLoc>();
	annot->insert_def(named_loc);
      } else if (location->isaUnknown()) {
	annot->insert_def(location);
      } else {
	rcc_error("Unexpected location type");
      }
    }  // next MOD location
    for(li = m_side_effect->getREFIterator(make_call_h(CAR(cs_c))); li->isValid(); ++(*li)) {
      OA_ptr<OA::Location> location; location = li->current();
      if (location->isaNamed()) {
	OA_ptr<NamedLoc> named_loc; named_loc = location.convert<NamedLoc>();
	annot->insert_use(named_loc);
      } else if (location->isaUnknown()) {
	annot->insert_use(location);
      } else {
	rcc_error("Unexpected location type");
      }
    }  // next REF location
  }  // next call site in expression
  
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
  // TODO: refine to include arithmetic, etc.
  return (is_const(e) || is_var(e));
}

// "cheaply evaluable" expressions take a "reasonably short" amount of
// time to evaluate. This is used for the call-by-value
// transformation: the transformation may be unprofitable if the
// callee is nonstrict in the formal argument and the corresponding
// actual argument is expensive to evaluate.
bool SideEffectAnnotationMap::expression_is_cheap(const SEXP e) {
  return (is_const(e) || is_var(e));
}

} // end namespace RAnnot
