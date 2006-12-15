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

#include <ParseInfo.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <support/DumpMacros.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/DefVar.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/HandleInterface.h>
#include <analysis/RequiresContext.h>
#include <analysis/StrictnessDFSolver.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#include "FuncInfo.h"

using namespace OA;
using namespace HandleInterface;
namespace RAnnot {

//****************************************************************************
// FuncInfo
//****************************************************************************

typedef FuncInfo::mention_iterator mention_iterator;
typedef FuncInfo::const_mention_iterator const_mention_iterator;
typedef FuncInfo::call_site_iterator call_site_iterator;
typedef FuncInfo::const_call_site_iterator const_call_site_iterator;

FuncInfo::FuncInfo(FuncInfo* parent, SEXP name, SEXP defn) :
  m_parent(parent),
  m_first_name(name),
  m_defn(defn),
  m_c_name(""),
  m_closure(""),
  NonUniformDegreeTreeNodeTmpl<FuncInfo>(parent)
{
  m_requires_context = functionRequiresContext(defn);

  // make formal argument annotations
  SEXP args = get_args();
  for (SEXP e = args; e != R_NilValue; e = CDR(e)) {
    FormalArgInfo * formal_info = new FormalArgInfo(e);
    putProperty(FormalArgInfo, e, formal_info, false);
  }

  // this is a new lexical scope
  m_scope = new FundefLexicalScope(defn);
}

FuncInfo::~FuncInfo()
{
}

unsigned int FuncInfo::get_num_args() const 
{
  return m_num_args;
}

void FuncInfo::set_num_args(unsigned int x) 
{
  m_num_args = x;
}

SEXP FuncInfo::get_defn()
{
  return m_defn;
}

SEXP FuncInfo::get_first_name()
{
  return m_first_name;
}

bool FuncInfo::get_has_var_args() const
{
  return m_has_var_args;
}

void FuncInfo::set_has_var_args(bool x)
{
  m_has_var_args = x;
}

void FuncInfo::set_requires_context(bool requires_context) 
{ 
  m_requires_context = requires_context; 
}

bool FuncInfo::requires_context() 
{ 
  return m_requires_context;
}

SEXP FuncInfo::get_args() const
{ 
  return CAR(fundef_args_c(m_defn)); 
}

bool FuncInfo::is_arg(SEXP sym)
{
  SEXP args = get_args();
  SEXP e;
  for (e = args; e != R_NilValue; e = CDR(e)) {
    if (TAG(e) == sym) return true;
  }
  return false;
}

int FuncInfo::find_arg_position(char* name)
{
  SEXP args = get_args();
  int pos = 1;
  SEXP e;
  for (e = args; e != R_NilValue; e = CDR(e), pos++) {
    char* arg_name = CHAR(PRINTNAME(INTERNAL(e)));
    if (!strcmp(arg_name, name)) break;
  }
  assert (e != R_NilValue);
  return pos;
}

SEXP FuncInfo::get_arg(int position)
{
  SEXP args = get_args();
  int p = 1;
  SEXP e;
  for (e = args; e != R_NilValue && p != position; e = CDR(e), p++);
  assert (e != R_NilValue);
  return e;
}

bool FuncInfo::is_arg_value(SEXP arg)
{
  FormalArgInfo* formal_info = getProperty(FormalArgInfo, arg);
  bool isvalue = formal_info->is_value();
  return isvalue;
}

bool FuncInfo::are_all_value()
{
  bool allvalue = true;
  SEXP args = get_args();
  for (SEXP e = args; e != R_NilValue && allvalue; e = CDR(e)) {
    if (!is_arg_value(e)) allvalue = false;
  }
  return allvalue;
}

const std::string& FuncInfo::get_c_name()
{
  if (m_c_name == "") {
    SEXP name_sym = get_first_name();
    if (name_sym == R_NilValue) {
      m_c_name = make_c_id("anon" + ParseInfo::global_fundefs->new_var_unp());
    } else {
      m_c_name = make_c_id(ParseInfo::global_fundefs->new_var_unp_name(var_name(name_sym)));
    }
  }
  return m_c_name;
}

const std::string& FuncInfo::get_closure()
{
  if (m_closure == "") {
    m_closure = ParseInfo::global_fundefs->new_sexp_unp();
  }
  return m_closure;
}

OA_ptr<CFG::Interface> FuncInfo::get_cfg() const
{
  return m_cfg;
}

void FuncInfo::set_cfg(OA_ptr<CFG::Interface> x)
{
  m_cfg = x;
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

FundefLexicalScope * FuncInfo::get_scope() const {
  return m_scope;
}

/// perform local function analysis (grabbing mentions and call sites) and strictness analysis
void FuncInfo::perform_analysis() {
  // compute CFG
  // pass 'true' as second arg to build statement-level CFG
  CFG::ManagerStandard cfg_man(R_Analyst::get_instance()->get_interface(), true);
  m_cfg = cfg_man.performAnalysis(make_proc_h(m_defn));

#if 0
  // moved to Analyst (collect_... needs VarAnnotationMap filled in,
  // which needs FuncInfos to traverse the scope tree) (ugh)

  collect_mentions_and_call_sites();
  analyze_args();

  // perform strictness analysis
  StrictnessDFSolver strict_solver(R_Analyst::get_instance()->get_interface());
  strict_solver.perform_analysis(make_proc_h(m_defn), m_cfg);
#endif
}

void FuncInfo::analyze_args() {
  SEXP args = CAR(fundef_args_c(m_defn));
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
    putProperty(Var, e, annot, true);
    if (TAG(e) == ddd) {
      has_var_args = true;
    }
  }
  set_num_args(n_args);
  set_has_var_args(has_var_args);
}

void FuncInfo::collect_mentions_and_call_sites() {
  assert(!m_cfg.ptrEqual(0));

  // for each node
  OA_ptr<CFG::Interface::NodesIterator> ni; ni = m_cfg->getNodesIterator();
  for( ; ni->isValid(); ++*ni) {
    // for each statement
    OA_ptr<CFG::Interface::NodeStatementsIterator> si;
    si = ni->current()->getNodeStatementsIterator();
    for( ; si->isValid(); ++*si) {
      // for each mention
      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, make_sexp(si->current()));
      assert(stmt_annot != 0);
      ExpressionInfo::const_var_iterator mi;
      for(mi = stmt_annot->begin_vars(); mi != stmt_annot->end_vars(); ++mi) {
	Var * v = getProperty(Var, (*mi)->getMention_c());
	// FIXME: should make sure we always get the data-flow-solved
	// version of the Var. Shouldn't have to loop through
	// getProperty!
	insert_mention(v);
      }
      ExpressionInfo::const_call_site_iterator cs;
      for(cs = stmt_annot->begin_call_sites(); cs != stmt_annot->end_call_sites(); ++cs) {
	insert_call_site(*cs);
      }
    }
  }
}

std::ostream& FuncInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FuncInfo);
  dumpPtr(os, this);
  dumpSEXP(os, m_first_name);
  dumpVar(os, m_num_args);
  dumpVar(os, m_has_var_args);
  dumpVar(os, m_c_name);
  dumpVar(os, m_requires_context);
  R_Analyst::get_instance()->dump_cfg(os, m_defn); // can't call CFG::dump; it requires the IRInterface
  dumpSEXP(os, m_defn);
  dumpPtr(os, m_parent);
  os << "Begin arguments:" << std::endl;
  for (SEXP arg = get_args(); arg != R_NilValue; arg = CDR(arg)) {
    FormalArgInfo * arg_annot = getProperty(FormalArgInfo, arg);
    arg_annot->dump(os);
  }
  os << "Begin mentions:" << std::endl;
  for (const_mention_iterator i = begin_mentions(); i != end_mentions(); ++i) {
    Var * v = *i;
    v->dump(os);
    VarBinding * vb = getProperty(VarBinding, (*i)->getMention_c());
    vb->dump(os);
  }
  os << "End mentions" << std::endl;
  endObjDump(os, FuncInfo);
}

PropertyHndlT FuncInfo::handle() {
  return FuncInfoAnnotationMap::handle();
}

}
