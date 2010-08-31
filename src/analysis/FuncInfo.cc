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
#include <analysis/StrictnessResult.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#include "FuncInfo.h"

using namespace OA;
using namespace HandleInterface;

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
  m_basic(basic),
  NonUniformDegreeTreeNodeTmpl<FuncInfo>(parent)
{
}

FuncInfo::~FuncInfo()
{
}

unsigned int FuncInfo::get_num_args() const 
{
  return m_basic->get_num_args();
}

void FuncInfo::set_num_args(unsigned int x) 
{
  m_basic->set_num_args(x);
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

void FuncInfo::set_has_var_args(bool x)
{
  m_basic->set_has_var_args(x);
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

bool FuncInfo::is_arg_value(SEXP arg) const
{
  return m_basic->is_arg_value(arg);
}

bool FuncInfo::are_all_value() const
{
  return m_basic->are_all_value();
}

const std::string& FuncInfo::get_c_name()
{
  return m_basic->get_c_name();
}

const std::string& FuncInfo::get_closure()
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

FundefLexicalScope * FuncInfo::get_scope() const {
  return m_basic->get_scope();
}

/// perform local function analysis (grabbing mentions and call sites) and strictness analysis
void FuncInfo::perform_analysis() {
  // TODO
}

BasicFuncInfo * FuncInfo::get_basic() {
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

bool FuncInfo::has_children() const {
  return m_basic->has_children();
}

OA_ptr<Strictness::StrictnessResult> FuncInfo::get_strictness() const {
  return m_strictness;
}

void FuncInfo::set_strictness(OA_ptr<Strictness::StrictnessResult> x) {
  m_strictness = x;
}

std::ostream& FuncInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FuncInfo);
  m_basic->dump(os);
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

FuncInfoIterator::FuncInfoIterator(const FuncInfo * fi, TraversalOrder torder, NonUniformDegreeTreeEnumType how)
  : NonUniformDegreeTreeIteratorTmpl<FuncInfo>(fi, torder, how)
{}

} // namespace RAnnot


