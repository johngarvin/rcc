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

// File: UseVar.cc
//
// An annotation representing a variable reference that is a use.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <include/R/R_RInternals.h>

#include <support/DumpMacros.h>

#include "UseVar.h"

namespace RAnnot {

typedef UseVar::PositionT PositionT;

//****************************************************************************
// UseVar
//****************************************************************************

UseVar::UseVar(SEXP mention_c, PositionT position, MayMustT mmt, Locality::LocalityType lt)
  : BasicVar(mention_c, BasicVar::Var_USE, mmt, lt),
    m_position_type(position)
{
}

UseVar::~UseVar()
{
}

SEXP UseVar::get_name() const
{
  return CAR(get_mention_c());
}

void UseVar::accept(VarVisitor * v)
{
  v->visitUseVar(this);
}

PositionT UseVar::get_position_type() const
{
  return m_position_type;
}
  
UseVar * UseVar::clone()
{
  return new UseVar(*this);
}

std::ostream & UseVar::dump(std::ostream & os) const
{
  beginObjDump(os, UseVar);
  //dumpSEXP(os,mSEXP);
  SEXP name = get_name();
  dumpSEXP(os, name);
  dumpName(os, get_use_def_type());
  dumpName(os, get_may_must_type());
  dumpName(os, get_basic_scope_type());
  dumpName(os, get_position_type());
  endObjDump(os, UseVar);
}

const std::string type_name(const UseVar::PositionT x)
{
  switch(x) {
  case UseVar::UseVar_FUNCTION: return "FUNCTION";
  case UseVar::UseVar_ARGUMENT: return "ARGUMENT";
  }
}

}
