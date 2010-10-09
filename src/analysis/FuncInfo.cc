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

// File: FuncInfo.cc
//
// Annotation for a function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/CFG/ManagerCFG.hpp>

#include <ParseInfo.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <support/DumpMacros.h>
#include <support/Debug.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/DebutDFSolver.h>
#include <analysis/DefVar.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/HandleInterface.h>
#include <analysis/RequiresContext.h>
#include <analysis/StrictnessDFSet.h>
#include <analysis/StrictnessDFSetIterator.h>
#include <analysis/StrictnessDFSolver.h>
#include <analysis/StrictnessResult.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#include "FuncInfo.h"

using namespace OA;
using namespace HandleInterface;

static bool debug;

namespace RAnnot {

//****************************************************************************
// FuncInfo
//****************************************************************************

static void accum_implicit_returns(SEXP cell);

typedef FuncInfo::mention_iterator mention_iterator;
typedef FuncInfo::const_mention_iterator const_mention_iterator;
typedef FuncInfo::call_site_iterator call_site_iterator;
typedef FuncInfo::const_call_site_iterator const_call_site_iterator;

  FuncInfo::FuncInfo(FuncInfo * parent, BasicFuncInfo * basic) :
  m_strictness(),
  m_mentions(),
  m_call_sites(),
  m_basic(basic),
  NonUniformDegreeTreeNodeTmpl<FuncInfo>(parent)
{
  RCC_DEBUG("RCC_FuncInfo", debug);
  collect_mentions_and_call_sites();
  analyze_strictness();
  analyze_debuts();
}

FuncInfo::~FuncInfo()
{
}

AnnotationBase * FuncInfo::clone()
{
  return 0;
}

unsigned int FuncInfo::get_num_args() const 
{
  return m_basic->get_num_args();
}

SEXP FuncInfo::get_sexp() const
{
  return m_basic->get_sexp();
}

SEXP FuncInfo::get_first_name_c() const
{
  return m_basic->get_first_name_c();
}

bool FuncInfo::get_has_var_args() const
{
  return m_basic->get_has_var_args();
}

bool FuncInfo::requires_context() const
{ 
  return m_basic->requires_context();
}

SEXP FuncInfo::get_args() const
{ 
  return m_basic->get_args();
}

bool FuncInfo::is_arg(SEXP sym) const
{
  return m_basic->is_arg(sym);
}

int FuncInfo::find_arg_position(char * name) const
{
  return m_basic->find_arg_position(name);
}

SEXP FuncInfo::get_arg(int position) const
{
  return m_basic->get_arg(position);
}

const std::string & FuncInfo::get_c_name()
{
  return m_basic->get_c_name();
}

const std::string & FuncInfo::get_closure() const
{
  return m_basic->get_closure();
}

OA_ptr<CFG::CFG> FuncInfo::get_cfg() const
{
  return m_basic->get_cfg();
}

// mention iterators

mention_iterator FuncInfo::begin_mentions()
{
  return m_mentions.begin();
}

const_mention_iterator FuncInfo::begin_mentions() const
{
  return m_mentions.begin();
}

mention_iterator FuncInfo::end_mentions()
{
  return m_mentions.end();
}

const_mention_iterator FuncInfo::end_mentions() const
{
  return m_mentions.end();
}

// call site iterators

call_site_iterator FuncInfo::begin_call_sites()
{
  return m_call_sites.begin();
}

const_call_site_iterator FuncInfo::begin_call_sites() const
{
  return m_call_sites.begin();
}

call_site_iterator FuncInfo::end_call_sites()
{
  return m_call_sites.end();
}

const_call_site_iterator FuncInfo::end_call_sites() const
{
  return m_call_sites.end();
}


void FuncInfo::insert_mention(FuncInfo::MentionT v)
{
  m_mentions.push_back(v);
}

void FuncInfo::insert_call_site(FuncInfo::CallSiteT cs)
{
  m_call_sites.push_back(cs);
}

const FundefLexicalScope * FuncInfo::get_scope() const
{
  return m_basic->get_scope();
}

BasicFuncInfo * FuncInfo::get_basic()
{
  return m_basic;
}

const std::set<SEXP> * FuncInfo::get_implicit_returns() const
{
  return m_basic->get_implicit_returns();
}

bool FuncInfo::is_return(SEXP cell) const
{
  return m_basic->is_return(cell);
}

SEXP FuncInfo::return_value_c(const SEXP cell) const
{
  return m_basic->return_value_c(cell);
}

bool FuncInfo::has_children() const
{
  return m_basic->has_children();
}

OA_ptr<Strictness::StrictnessResult> FuncInfo::get_strictness() const
{
  return m_strictness;
}

void FuncInfo::set_strictness(OA_ptr<Strictness::StrictnessResult> x)
{
  m_strictness = x;
}

std::ostream & FuncInfo::dump(std::ostream & os) const
{
  beginObjDump(os, FuncInfo);
  m_basic->dump(os);
  os << "Begin mentions:" << std::endl;
  for (const_mention_iterator i = begin_mentions(); i != end_mentions(); ++i) {
    Var * v = getProperty(Var, *i);
    v->dump(os);
    VarBinding * vb = getProperty(VarBinding, *i);
    vb->dump(os);
  }
  os << "End mentions" << std::endl;
  endObjDump(os, FuncInfo);
}

PropertyHndlT FuncInfo::handle() {
  return FuncInfoAnnotationMap::handle();
}

/// Find each mention (use or def) and call site in the function
void FuncInfo::collect_mentions_and_call_sites() {
  OA_ptr<CFG::NodeInterface> node;
  StmtHandle stmt;
  SEXP use, def, cs;

  PROC_FOR_EACH_NODE(this, node) {
    NODE_FOR_EACH_STATEMENT(node, stmt) {
      // for each mention
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(stmt));
      if (debug) {
	std::cerr << "FuncInfo collecting mentions/callsites for statement: ";
	stmt_annot->dump(std::cerr);
      }
      assert(stmt_annot != 0);
      EXPRESSION_FOR_EACH_USE(stmt_annot, use) {
	insert_mention(use);
      }
      EXPRESSION_FOR_EACH_DEF(stmt_annot, def) {
	insert_mention(def);
      }
      EXPRESSION_FOR_EACH_CALL_SITE(stmt_annot, cs) {
	insert_call_site(cs);
      }
    }
  }
}

// solve the strictness data flow problem and update the information
// in the formal argument annotations
void FuncInfo::analyze_strictness() {
  assert(!get_cfg().ptrEqual(0));
  Strictness::StrictnessDFSolver strict_solver(R_Analyst::instance()->get_interface());
  OA_ptr<Strictness::StrictnessResult> strict; strict = strict_solver.perform_analysis(make_proc_h(get_sexp()), get_cfg());
  set_strictness(strict);
  OA_ptr<Strictness::DFSet> strict_set = strict->get_args_on_exit();
  if (debug) strict_set->dump(std::cout, R_Analyst::instance()->get_interface());
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
void FuncInfo::analyze_debuts() {
  OA_ptr<CFG::CFGInterface> cfg; cfg = get_cfg();
  assert(!cfg.ptrEqual(0));
  DebutDFSolver debut_solver(R_Analyst::instance()->get_interface());
  OA_ptr<NameMentionMultiMap> debut_map = debut_solver.perform_analysis(this);
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


// ----- FuncInfoIterator -----

FuncInfoIterator::FuncInfoIterator(const FuncInfo * fi, TraversalOrder torder, NonUniformDegreeTreeEnumType how)
  : NonUniformDegreeTreeIteratorTmpl<FuncInfo>(fi, torder, how)
{}

} // namespace RAnnot
