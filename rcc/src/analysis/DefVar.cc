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

// File: DefVar.h
//
// Annotation for a variable mention that is a def.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <assert.h>

#include <support/DumpMacros.h>

#include "DefVar.h"

namespace RAnnot {

typedef DefVar::SourceT SourceT;

DefVar::DefVar(SEXP mention_c, SourceT source, MayMustT mmt, Locality::LocalityType lt, SEXP rhs_c)
  : BasicVar(mention_c, Var_DEF, mmt, lt),
    m_source_type(source),
    m_rhs_c(rhs_c)
{
}

DefVar::~DefVar()
{
}

SourceT DefVar::get_source_type() const
{
  return m_source_type;
}

SEXP DefVar::get_name() const
{
  if (m_source_type == DefVar_ASSIGN) {
    return CAR(get_mention_c());
  } else if (m_source_type == DefVar_FORMAL) {
    return TAG(get_mention_c());
  } else {
    assert(0);
    return R_NilValue;
  }
}

SEXP DefVar::get_rhs_c() const
{
  return m_rhs_c;
}

void DefVar::accept(VarVisitor * v)
{
  v->visitDefVar(this);
}

DefVar * DefVar::clone() {
  return new DefVar(*this);
}

std::ostream & DefVar::dump(std::ostream & os) const
{
  beginObjDump(os,DefVar);
  //dumpSEXP(os,mSEXP);
  SEXP name = get_name();
  dumpSEXP(os, name);
  dumpName(os, get_use_def_type());
  dumpName(os, get_may_must_type());
  dumpName(os, get_basic_scope_type());
  dumpName(os, get_source_type());
  endObjDump(os, DefVar);
}

const std::string type_name(const DefVar::SourceT x)
{
  switch(x) {
  case DefVar::DefVar_ASSIGN: return "ASSIGN";
  case DefVar::DefVar_FORMAL: return "FORMAL";
  case DefVar::DefVar_PHI:    return "PHI";
  }
}

}
