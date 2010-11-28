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

// File: LibraryFuncInfo.cc
//
// Annotation for a closure in R's library.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <cassert>

#include <ParseInfo.h>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <support/DumpMacros.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisResults.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/RequiresContext.h>
#include <analysis/LibraryFuncInfoAnnotationMap.h>
#include <analysis/Utils.h>

#include "LibraryFuncInfo.h"

namespace RAnnot {

LibraryFuncInfo::LibraryFuncInfo(SEXP name_c, SEXP sexp) :
  m_c_name(""),
  m_closure(ParseInfo::global_fundefs->new_sexp_unp()),
  m_requires_context(functionRequiresContext(sexp)),
  m_sexp(sexp),
  m_first_name_c(name_c)
{
  perform_analysis();
}

LibraryFuncInfo::~LibraryFuncInfo()
{
}

AnnotationBase * LibraryFuncInfo::clone()
{
  return 0;  // should not be cloned
}

PropertyHndlT LibraryFuncInfo::handle()
{
  return LibraryFuncInfoAnnotationMap::handle();
}

unsigned int LibraryFuncInfo::get_num_args() const 
{
  return m_num_args;
}

SEXP LibraryFuncInfo::get_sexp() const
{
  return m_sexp;
}

SEXP LibraryFuncInfo::get_first_name_c() const
{
  return m_first_name_c;
}

bool LibraryFuncInfo::get_has_var_args() const
{
  return m_has_var_args;
}

bool LibraryFuncInfo::requires_context() const
{ 
  return m_requires_context;
}

SEXP LibraryFuncInfo::get_args() const
{ 
  return procedure_args(m_sexp);
}

bool LibraryFuncInfo::is_arg(SEXP sym) const
{
  SEXP args = get_args();
  SEXP e;
  for (e = args; e != R_NilValue; e = CDR(e)) {
    if (TAG(e) == sym) return true;
  }
  return false;
}

int LibraryFuncInfo::find_arg_position(char * name) const
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

int LibraryFuncInfo::find_arg_position(std::string name) const {
  return find_arg_position(name.c_str());
}

SEXP LibraryFuncInfo::get_arg(int position) const
{
  SEXP args = get_args();
  int p = 1;
  SEXP e;
  for (e = args; e != R_NilValue && p != position; e = CDR(e), p++);
  assert (e != R_NilValue);
  return e;
}

const std::string & LibraryFuncInfo::get_c_name()
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

const std::string & LibraryFuncInfo::get_closure() const
{
  return m_closure;
}

void LibraryFuncInfo::perform_analysis() {
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
}

std::ostream & LibraryFuncInfo::dump(std::ostream & os) const
{
  beginObjDump(os, LibraryFuncInfo);
  dumpPtr(os, this);
  SEXP name = CAR(m_first_name_c);
  dumpSEXP(os, name);
  dumpVar(os, m_num_args);
  dumpVar(os, m_has_var_args);
  dumpVar(os, m_c_name);
  dumpVar(os, m_requires_context);
  dumpSEXP(os, m_sexp);
  os << "Begin arguments:" << std::endl;
  for (SEXP arg = get_args(); arg != R_NilValue; arg = CDR(arg)) {
    FormalArgInfo * arg_annot = getProperty(FormalArgInfo, arg);
    arg_annot->dump(os);
  }
  endObjDump(os, LibraryFuncInfo);
}

} // end namespace RAnnot
