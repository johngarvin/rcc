// -*- Mode: C++ -*-
//
// Copyright (c) 2006 Rice University
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

// File: LocalFunctionAnalysis.cc
//
// Discovers local information about the given function
// definition. Adds annotations to the global AnalysisResults.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/DefaultDFSet.h>
#include <analysis/DefVar.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/DebutDFSolver.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/LocalityType.h>
#include <analysis/NameMentionMultiMap.h>
#include <analysis/StrictnessDFSetIterator.h>
#include <analysis/StrictnessDFSolver.h>
#include <analysis/StrictnessResult.h>
#include <analysis/Utils.h>
#include <analysis/Var.h>

#include "LocalFunctionAnalysis.h"

using namespace OA;
using namespace RAnnot;
using namespace HandleInterface;

static bool debug;

LocalFunctionAnalysis::LocalFunctionAnalysis(FuncInfo * const fi)
  : m_fi(fi)
{
  assert(fi != 0);
  RCC_DEBUG("RCC_LocalFunctionAnalysis", debug);
}

/// Discovers local information about the given function
/// definition. Adds annotations to the global AnalysisResults.
void LocalFunctionAnalysis::perform_analysis() {
  analyze_args();
  collect_mentions_and_call_sites();
  analyze_strictness();
  analyze_debuts();
}

void LocalFunctionAnalysis::analyze_args() {
  SEXP args = m_fi->get_args();
  const SEXP ddd = Rf_install("...");
  bool has_var_args = false;
  int n_args = 0;
  for(SEXP e = args; e != R_NilValue; e = CDR(e)) {
    ++n_args;
    DefVar * annot = new DefVar();
    annot->setMention_c(e);
    annot->setSourceType(DefVar::DefVar_FORMAL);
    annot->setMayMustType(Var::Var_MUST);
    annot->setScopeType(Locality::Locality_LOCAL);
    annot->setRhs_c(0);
    putProperty(Var, e, annot);
    if (TAG(e) == ddd) {
      has_var_args = true;
    }
  }
  m_fi->set_num_args(n_args);
  m_fi->set_has_var_args(has_var_args);
}

/// Find each mention (use or def) and call site in the function
void LocalFunctionAnalysis::collect_mentions_and_call_sites() {
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;
  SEXP cs;
  UseVar * use;
  DefVar * def;

  PROC_FOR_EACH_NODE(m_fi, node) {
    NODE_FOR_EACH_STATEMENT(node, stmt) {
      // for each mention
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(si->current()));
      assert(stmt_annot != 0);
      EXPRESSION_FOR_EACH_USE(stmt_annot, use) {
	m_fi->insert_mention(use);
      }
      EXPRESSION_FOR_EACH_DEF(stmt_annot, def) {
	m_fi->insert_mention(def);
      }
      EXPRESSION_FOR_EACH_CALL_SITE(stmt_annot, cs) {
	m_fi->insert_call_site(cs);
      }
    }
  }
}

// solve the strictness data flow problem and update the information
// in the formal argument annotations
void LocalFunctionAnalysis::analyze_strictness() {
  OA_ptr<CFG::CFGInterface> cfg; cfg = m_fi->get_cfg();
  assert(!cfg.ptrEqual(0));
  Strictness::StrictnessDFSolver strict_solver(R_Analyst::get_instance()->get_interface());
  OA_ptr<Strictness::StrictnessResult> strict; strict = strict_solver.perform_analysis(make_proc_h(m_fi->get_sexp()), cfg);
  OA_ptr<Strictness::DFSet> strict_set = strict->get_args_on_exit();
  if (debug) strict_set->dump(std::cout, R_Analyst::get_instance()->get_interface());
  OA_ptr<Strictness::DFSetIterator> it = strict_set->get_iterator();
  for (it->reset(); it->isValid(); ++*it) {
    FormalArgInfo * annot = getProperty(FormalArgInfo, it->current()->get_loc()->get_sexp());
    if (debug) annot->dump(std::cerr);
    if (it->current()->get_strictness_type() == Strictness::Strictness_USED) {
      if (debug) std::cerr << "arg is strict" << std::endl;
      annot->set_is_strict(true);
    }
  }
}

// find the set of debuts of each name (those that are the
// first mention of that name on some path)
void LocalFunctionAnalysis::analyze_debuts() {
  OA_ptr<CFG::CFGInterface> cfg; cfg = m_fi->get_cfg();
  assert(!cfg.ptrEqual(0));
  DebutDFSolver debut_solver(R_Analyst::get_instance()->get_interface());
  OA_ptr<NameMentionMultiMap> debut_map = debut_solver.perform_analysis(make_proc_h(m_fi->get_sexp()), cfg);
  typedef NameMentionMultiMap::const_iterator Iterator;
  for (Iterator name = debut_map->begin(); name != debut_map->end(); name++) {
    Iterator start = debut_map->lower_bound(name->first);
    Iterator end = debut_map->upper_bound(name->first);
    for(Iterator mi = start; mi != end; mi++) {
      Var * annot = getProperty(Var, name->first);
      annot->set_first_on_some_path(true);
    }
  }
}
