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

// File: Analyst.cc
//
// Singleton manager for annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <algorithm>
#include <set>

#include <ParseInfo.h>

#include <support/RccError.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/BasicFuncInfo.h>
#include <analysis/CallByValueAnalysis.h>
#include <analysis/EscapedDFSolver.h>
#include <analysis/ExpressionInfoAnnotationMap.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/LocalFunctionAnalysis.h>
#include <analysis/NameBoolDFSet.h>
#include <analysis/OEscapeDFSolver.h>
#include <analysis/ReturnedDFSolver.h>
#include <analysis/SimpleIterators.h>
#include <analysis/Utils.h>
#include <analysis/VarAnnotationMap.h>

#include "Analyst.h"

using namespace RAnnot;
using namespace RProp;
using namespace HandleInterface;

// ----- implement Singleton pattern -----

/// the only instance
R_Analyst * R_Analyst::s_instance = 0;

/// static instantiation
R_Analyst * R_Analyst::get_instance(SEXP _program) {
  if (s_instance == 0) {
    s_instance = new R_Analyst(_program);
  }
  return s_instance;
}

/// just get the existing instance; error if not yet instantiated
R_Analyst * R_Analyst::get_instance() {
  if (s_instance == 0) {
    rcc_error("R_Analyst is not yet instantiated");
  }
  return s_instance;
}

// ----- constructor -----

/// construct an R_Analyst by providing an SEXP representing the whole program
R_Analyst::R_Analyst(SEXP _program)
  : m_program(_program),
    m_interface(),
    m_global_scope(0)
  {}

// ----- analysis -----

void R_Analyst::perform_analysis() {
  // initialize what's not initialized by the constructor
  m_interface = new R_IRInterface();
  m_global_scope = FuncInfoAnnotationMap::get_instance()->get_scope_tree_root()->get_scope();
  
  if (ParseInfo::allow_oo()           ||
      ParseInfo::allow_envir_manip())
    {
      throw AnalysisException("Unable to perform analysis: no assertion to exclude OO or environment manipulation");
    }
  build_local_function_info();
  (new CallByValueAnalysis())->perform_analysis();
}

/// Discovers local information on procedures: arguments, names
/// mentioned, etc.
void R_Analyst::build_local_function_info() {
  FuncInfo * fi;
  FOR_EACH_PROC_AND_LIB(fi) {
    LocalFunctionAnalysis lfa(fi);
    lfa.perform_analysis();
  }
  collect_libraries();
}

void R_Analyst::collect_libraries() {
  FuncInfo * fi;
  std::set<SEXP> libs;
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_CALL_SITE(fi, csi) {
      SEXP lhs = call_lhs(CAR(*csi));
      if (is_var(lhs) && is_library(lhs) && is_closure(library_value(lhs))) {
	BasicFuncInfo * new_bfi; new_bfi = new BasicFuncInfo(0, library_value(lhs), library_value(lhs));
	new_bfi->perform_analysis();
	putProperty(BasicFuncInfo, library_value(lhs), new_bfi);
	FuncInfo * new_fi; new_fi = new FuncInfo(0, new_bfi);
	new_fi->perform_analysis();
	putProperty(FuncInfo, library_value(lhs), new_fi);
	libs.insert(library_value(lhs));
      }
    }
  }
  for(std::set<SEXP>::const_iterator iter = libs.begin(); iter != libs.end(); ++iter) {
    FuncInfo * fi = getProperty(FuncInfo, *iter);
    OA::OA_ptr<OA::CFG::NodeInterface> node;
    OA::StmtHandle stmt;
    PROC_FOR_EACH_NODE(fi, node) {
      NODE_FOR_EACH_STATEMENT(node, stmt) {
	ExpressionInfoAnnotationMap::get_instance()->make_annot(make_sexp(stmt));
      }
    }
    VarAnnotationMap::get_instance()->compute_proc(fi);
  }
}

LexicalScope * R_Analyst::get_library_scope() {
  return InternalLexicalScope::get_instance();
}

LexicalScope * R_Analyst::get_global_scope() {
  return m_global_scope;
}

/// Dump the CFG of the given function definition. Located here in
/// R_Analyst because cfg->dump needs the IRInterface as an argument.
void R_Analyst::dump_cfg(std::ostream &os, SEXP proc) {
  if (proc != R_NilValue) {
    FuncInfo * fi = getProperty(FuncInfo, proc);
    OA::OA_ptr<OA::CFG::CFG> cfg; cfg = fi->get_cfg();
    cfg->dump(os, m_interface);
  }
}

void R_Analyst::dump_all_cfgs(std::ostream &os) {
  FuncInfo * fi;
  FOR_EACH_PROC(fi) {
    OA::OA_ptr<OA::CFG::CFG> cfg; cfg = fi->get_cfg();
    cfg->dump(os, m_interface);
  }
}
