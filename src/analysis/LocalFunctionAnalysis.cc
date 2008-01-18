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

void collect_mentions_and_call_sites(OA_ptr<CFG::CFGInterface> cfg);

static const bool debug = false;

LocalFunctionAnalysis::LocalFunctionAnalysis(const SEXP fundef)
  : m_fundef(fundef)
{
  assert(is_fundef(fundef));
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
  SEXP args = CAR(fundef_args_c(m_fundef));
  const SEXP ddd = Rf_install("...");
  bool has_var_args = false;
  int n_args = 0;
  for(SEXP e = args; e != R_NilValue; e = CDR(e)) {
    ++n_args;
    DefVar * annot = new DefVar();
    annot->setMention_c(e);
    annot->setSourceType(DefVar::DefVar_FORMAL);
    annot->setMayMustType(Var::Var_MUST);
    annot->setScopeType(Locality_LOCAL);
    annot->setRhs_c(0);
    putProperty(Var, e, annot);
    if (TAG(e) == ddd) {
      has_var_args = true;
    }
  }
  FuncInfo * fi = getProperty(FuncInfo, m_fundef);
  assert(fi != 0);
  fi->set_num_args(n_args);
  fi->set_has_var_args(has_var_args);
}

/// Find each mention (use or def) and call site in the function
void LocalFunctionAnalysis::collect_mentions_and_call_sites() {
  FuncInfo * fi = getProperty(FuncInfo, m_fundef);
  assert(fi != 0);
  OA_ptr<CFG::CFGInterface> cfg; cfg = fi->get_cfg();
  assert(!cfg.ptrEqual(0));
  // for each node
  OA_ptr<CFG::NodesIteratorInterface> ni; ni = cfg->getCFGNodesIterator();
  for( ; ni->isValid(); ++*ni) {
    // for each statement
    OA_ptr<CFG::NodeStatementsIteratorInterface> si;
    si = ni->current().convert<CFG::NodeInterface>()->getNodeStatementsIterator();
    for( ; si->isValid(); ++*si) {
      // for each mention
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(si->current()));
      assert(stmt_annot != 0);
      ExpressionInfo::const_var_iterator mi;
      for(mi = stmt_annot->begin_vars(); mi != stmt_annot->end_vars(); ++mi) {
	Var * v = getProperty(Var, (*mi)->getMention_c());
	// TODO: should make sure we always get the data-flow-solved
	// version of the Var. Shouldn't have to loop through
	// getProperty!
	fi->insert_mention(v);
      }
      ExpressionInfo::const_call_site_iterator cs;
      for(cs = stmt_annot->begin_call_sites(); cs != stmt_annot->end_call_sites(); ++cs) {
	fi->insert_call_site(*cs);
      }
    }
  }
}

// solve the strictness data flow problem and update the information
// in the formal argument annotations
void LocalFunctionAnalysis::analyze_strictness() {
  FuncInfo * fi = getProperty(FuncInfo, m_fundef);
  assert(fi != 0);
  OA_ptr<CFG::CFGInterface> cfg; cfg = fi->get_cfg();
  assert(!cfg.ptrEqual(0));
  assert(m_fundef != 0);
  Strictness::StrictnessDFSolver strict_solver(R_Analyst::get_instance()->get_interface());
  OA_ptr<Strictness::StrictnessResult> strict; strict = strict_solver.perform_analysis(make_proc_h(m_fundef), cfg);
  OA_ptr<Strictness::DFSet> strict_set = strict->get_args_on_exit();
  if (debug) strict_set->dump(std::cout, R_Analyst::get_instance()->get_interface());
  OA_ptr<Strictness::DFSetIterator> it = strict_set->get_iterator();
  for (it->reset(); it->isValid(); ++*it) {
    FormalArgInfo * annot = getProperty(FormalArgInfo, it->current()->get_loc()->get_sexp());
    if (it->current()->get_strictness_type() == Strictness_USED) {
      annot->set_is_strict(true);
    }
  }
}

// find the set of debuts of each name (those that are the
// first mention of that name on some path)
void LocalFunctionAnalysis::analyze_debuts() {
  FuncInfo * fi = getProperty(FuncInfo, m_fundef);
  assert(fi != 0);
  OA_ptr<CFG::CFGInterface> cfg; cfg = fi->get_cfg();
  assert(!cfg.ptrEqual(0));
  DebutDFSolver debut_solver(R_Analyst::get_instance()->get_interface());
  OA_ptr<NameMentionMultiMap> debut_map = debut_solver.perform_analysis(make_proc_h(m_fundef), cfg);
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
