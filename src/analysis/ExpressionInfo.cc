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

// File: ExpressionInfo.cc
//
// Annotation for expressions.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/DumpMacros.h>

#include <analysis/ExpressionInfoAnnotationMap.h>
#include <analysis/Var.h>

#include "ExpressionInfo.h"

namespace RAnnot {

//****************************************************************************
// ExpressionInfo
//****************************************************************************

ExpressionInfo::ExpressionInfo()
{
}


ExpressionInfo::~ExpressionInfo()
{
}


std::ostream&
ExpressionInfo::dump(std::ostream& os) const
{
  beginObjDump(os, ExpressionInfo);
  SEXP definition = CAR(mDefn);
  dumpSEXP(os, definition);

  os << "Begin mentions:" << std::endl;
  const_var_iterator var_iter;
  for(var_iter = mVars.begin(); var_iter != mVars.end(); ++var_iter) {
    (*var_iter)->dump(os);
  }
  os << "End mentions" << std::endl;

  os << "Begin call sites:" << std::endl;
  const_call_site_iterator cs_iter;
  for(cs_iter = mCallSites.begin(); cs_iter != mCallSites.end(); ++cs_iter) {
    dumpSEXP(os, *cs_iter);
  }
  os << "End call sites" << std::endl;

  endObjDump(os, ExpressionInfo);
}

PropertyHndlT
ExpressionInfo::handle() {
  return ExpressionInfoAnnotationMap::handle();
}

} // end namespace RAnnot
