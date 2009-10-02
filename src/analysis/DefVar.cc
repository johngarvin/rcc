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

DefVar::DefVar()
{
  mUseDefType = Var_DEF;
}

DefVar::~DefVar()
{
}

SEXP DefVar::getName() const
{
  if (mSourceType == DefVar_ASSIGN) {
    return CAR(mSEXP);
  } else if (mSourceType == DefVar_FORMAL) {
    return TAG(mSEXP);
  } else {
    assert(0);
    return R_NilValue;
  }
}

void DefVar::accept(VarVisitor * v) {
  v->visitDefVar(this);
}

std::ostream&
DefVar::dump(std::ostream& os) const
{
  beginObjDump(os,DefVar);
  //dumpSEXP(os,mSEXP);
  SEXP name = getName();
  dumpSEXP(os, name);
  dumpName(os, mUseDefType);
  dumpName(os, mMayMustType);
  dumpName(os, mScopeType);
  dumpName(os, mSourceType);
  dumpVar(os, m_first_on_some_path);
  endObjDump(os,DefVar);
}

const std::string typeName(const DefVar::SourceT x)
{
  switch(x) {
  case DefVar::DefVar_ASSIGN: return "ASSIGN";
  case DefVar::DefVar_FORMAL: return "FORMAL";
  case DefVar::DefVar_PHI:    return "PHI";
  }
}

}
