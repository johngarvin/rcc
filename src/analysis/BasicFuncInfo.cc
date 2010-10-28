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

// File: BasicFuncInfo.cc
//
// Annotation for a function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/CFG/ManagerCFG.hpp>

#include <ParseInfo.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <support/DumpMacros.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/BasicFuncInfoAnnotationMap.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/RequiresContext.h>
#include <analysis/Utils.h>

#include "BasicFuncInfo.h"

using namespace OA;
using namespace HandleInterface;

namespace RAnnot {

  // ----- static function -----

static void accum_implicit_returns(SEXP cell);

  // ----- typedefs for readability -----

typedef FuncInfo::mention_iterator mention_iterator;
typedef FuncInfo::const_mention_iterator const_mention_iterator;
typedef FuncInfo::call_site_iterator call_site_iterator;
typedef FuncInfo::const_call_site_iterator const_call_site_iterator;

BasicFuncInfo::BasicFuncInfo(BasicFuncInfo * parent, SEXP name_c, SEXP sexp) :
  NonUniformDegreeTreeNodeTmpl<BasicFuncInfo>(parent),
  m_c_name(""),
  m_closure(ParseInfo::global_fundefs->new_sexp_unp()),
  m_requires_context(functionRequiresContext(sexp)),
  m_scope(new FundefLexicalScope(sexp)),
  m_sexp(sexp),
  m_first_name_c(name_c),
  m_parent(parent),
  m_returns()
{
  perform_analysis();
}

BasicFuncInfo::~BasicFuncInfo()
{
}

AnnotationBase * BasicFuncInfo::clone()
{
  return 0;  // should not be cloned
}

PropertyHndlT BasicFuncInfo::handle()
{
  return BasicFuncInfoAnnotationMap::handle();
}

unsigned int BasicFuncInfo::get_num_args() const 
{
  return m_num_args;
}

SEXP BasicFuncInfo::get_sexp() const
{
  return m_sexp;
}

SEXP BasicFuncInfo::get_first_name_c() const
{
  return m_first_name_c;
}

bool BasicFuncInfo::get_has_var_args() const
{
  return m_has_var_args;
}

bool BasicFuncInfo::requires_context() const
{ 
  return m_requires_context;
}

SEXP BasicFuncInfo::get_args() const
{ 
  return procedure_args(m_sexp);
}

bool BasicFuncInfo::is_arg(SEXP sym) const
{
  SEXP args = get_args();
  SEXP e;
  for (e = args; e != R_NilValue; e = CDR(e)) {
    if (TAG(e) == sym) return true;
  }
  return false;
}

int BasicFuncInfo::find_arg_position(char * name) const
{
  SEXP args = get_args();
  int pos = 1;
  SEXP e;
  for (e = args; e != R_NilValue; e = CDR(e), pos++) {
    const char * arg_name = var_name(INTERNAL(e)).c_str();
    if (!strcmp(arg_name, name)) break;
  }
  assert (e != R_NilValue);
  return pos;
}

int BasicFuncInfo::find_arg_position(std::string name) const {
  return find_arg_position(name.c_str());
}

SEXP BasicFuncInfo::get_arg(int position) const
{
  SEXP args = get_args();
  int p = 1;
  SEXP e;
  for (e = args; e != R_NilValue && p != position; e = CDR(e), p++);
  assert (e != R_NilValue);
  return e;
}

const std::string & BasicFuncInfo::get_c_name()
{
  if (m_c_name == "") {
    SEXP name_sym = CAR(get_first_name_c());
    if (name_sym == R_NilValue) {
      m_c_name = make_c_id("anon" + ParseInfo::global_fundefs->new_var_unp());
    } else {
      m_c_name = make_c_id(ParseInfo::global_fundefs->new_var_unp_name(var_name(name_sym)));
    }
  }
  return m_c_name;
}

const std::string & BasicFuncInfo::get_closure() const
{
  return m_closure;
}

OA_ptr<CFG::CFG> BasicFuncInfo::get_cfg() const
{
  return m_cfg;
}

const FundefLexicalScope * BasicFuncInfo::get_scope() const
{
  return m_scope;
}

const BasicFuncInfo * BasicFuncInfo::get_parent() const
{
  return m_parent;
}

/// perform local function analysis
void BasicFuncInfo::perform_analysis() {
  // get number of args and whether "..." is present
  static const SEXP ddd = Rf_install("...");
  m_has_var_args = false;
  m_num_args = 0;
  for(SEXP e = get_args(); e != R_NilValue; e = CDR(e)) {
    m_num_args++;
    if (TAG(e) == ddd) {
      m_has_var_args = true;
    }
  }

  // compute CFG
  // pass 'true' as second arg to build statement-level CFG
  CFG::ManagerCFGStandard cfg_man(R_Analyst::instance()->get_interface(), true);
  m_cfg = cfg_man.performAnalysis(make_proc_h(m_sexp));

  // find all explicit and implicit returns
  if (is_fundef(m_sexp)) {
    accum_implicit_returns(fundef_body_c(m_sexp));
  }
}

const std::set<SEXP> * BasicFuncInfo::get_implicit_returns() const {
  return &m_returns;
}

bool BasicFuncInfo::is_return(SEXP cell) const {
  assert(is_cons(cell));
  SEXP e = CAR(cell);
  return (is_explicit_return(e) || (m_returns.find(cell) != m_returns.end()));
}

SEXP BasicFuncInfo::return_value_c(const SEXP cell) const {
  assert(is_return(cell));
  SEXP e = CAR(cell);
  if (is_explicit_return(e)) {
    if (CDR(e) == R_NilValue) {
      return R_NilValue;
    } else {
      return call_nth_arg_c(e,1);
    }
  } else {
    return cell;
  }
}

void BasicFuncInfo::accum_implicit_returns(SEXP cell) {
  SEXP e = CAR(cell);
  if (is_curly_list(e)) {
    SEXP last_c = curly_body(e);
    while (CDR(last_c) != R_NilValue) {
      last_c = CDR(last_c);
    }
    accum_implicit_returns(last_c);
  } else if (is_if(e)) {
    accum_implicit_returns(if_truebody_c(e));
    accum_implicit_returns(if_falsebody_c(e));
  } else if (is_loop(e)) {
    accum_implicit_returns(loop_body_c(e));
  } else {
    m_returns.insert(cell);
  }
}

bool BasicFuncInfo::has_children() const {
  return !(ChildCount() == 0);
}

std::ostream & BasicFuncInfo::dump(std::ostream & os) const
{
  beginObjDump(os, BasicFuncInfo);
  dumpPtr(os, this);
  SEXP name = CAR(m_first_name_c);
  dumpSEXP(os, name);
  dumpVar(os, m_num_args);
  dumpVar(os, m_has_var_args);
  dumpVar(os, m_c_name);
  dumpVar(os, m_requires_context);
  R_Analyst::instance()->dump_cfg(os, m_sexp); // can't call CFG::dump; it requires the IRInterface
  dumpSEXP(os, m_sexp);
  dumpPtr(os, m_parent);
  os << "Begin arguments:" << std::endl;
  for (SEXP arg = get_args(); arg != R_NilValue; arg = CDR(arg)) {
    FormalArgInfo * arg_annot = getProperty(FormalArgInfo, arg);
    arg_annot->dump(os);
  }
  endObjDump(os, BasicFuncInfo);
}

// ----- BasicFuncInfoIterator -----

BasicFuncInfoIterator::BasicFuncInfoIterator(const BasicFuncInfo * bfi, TraversalOrder torder, NonUniformDegreeTreeEnumType how)
  : NonUniformDegreeTreeIteratorTmpl<BasicFuncInfo>(bfi, torder, how)
{}


}  // end namespace RAnnot

const OA_ptr<CFG::NodeInterface> RAnnot::BasicFuncInfo::iterator_dummy_node = OA_ptr<CFG::Node>();

