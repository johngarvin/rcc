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

//****************************************************************************
// UseVar
//****************************************************************************

UseVar::UseVar()
{
  mUseDefType = Var_USE;
}

UseVar::~UseVar()
{
}

SEXP UseVar::getName() const
{
  return CAR(mSEXP);
}

void UseVar::accept(VarVisitor * v) {
  v->visitUseVar(this);
}

std::ostream&
UseVar::dump(std::ostream& os) const
{
  beginObjDump(os,UseVar);
  //dumpSEXP(os,mSEXP);
  SEXP name = getName();
  dumpSEXP(os, name);
  dumpName(os, mUseDefType);
  dumpName(os, mMayMustType);
  dumpName(os, mScopeType);
  dumpName(os, mPositionType);
  dumpVar(os, m_first_on_some_path);
  endObjDump(os,UseVar);
}

const std::string typeName(const Var::UseDefT x)
{
  switch(x) {
  case Var::Var_USE: return "USE";
  case Var::Var_DEF: return "DEF";
  }
}

const std::string typeName(const UseVar::PositionT x)
{
  switch(x) {
  case UseVar::UseVar_FUNCTION: return "FUNCTION";
  case UseVar::UseVar_ARGUMENT: return "ARGUMENT";
  }
}

}
