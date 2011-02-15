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

// File: BasicVar.cc
//
// Basic annotation for a variable reference (includes uses and defs)
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/BasicVarAnnotationMap.h>

#include "BasicVar.h"

namespace RAnnot {

typedef BasicVar::UseDefT UseDefT;
typedef BasicVar::MayMustT MayMustT;

//****************************************************************************
// BasicVar
//****************************************************************************

BasicVar::BasicVar(SEXP sexp, UseDefT udt, MayMustT mmt, Locality::LocalityType lt)
  : m_sexp(sexp),
    m_use_def_type(udt),
    m_may_must_type(mmt),
    m_basic_scope_type(lt)
{
}


BasicVar::~BasicVar()
{
}

UseDefT BasicVar::get_use_def_type() const
{
  return m_use_def_type;
}

// may/must type
MayMustT BasicVar::get_may_must_type() const 
{
  return m_may_must_type;
}

// scope type
Locality::LocalityType BasicVar::get_basic_scope_type() const
{
  return m_basic_scope_type;
}

// Mention (cons cell that contains the name)
SEXP BasicVar::get_mention_c() const
{
  return m_sexp;
}

std::ostream & BasicVar::dump(std::ostream & os) const
{
  beginObjDump(os, BasicVar);
  endObjDump(os, BasicVar);
}

PropertyHndlT BasicVar::handle() {
  return BasicVarAnnotationMap::handle();
}

const std::string type_name(const BasicVar::UseDefT x)
{
  switch(x) {
  case BasicVar::Var_USE: return "USE";
  case BasicVar::Var_DEF: return "DEF";
  }
}

const std::string type_name(const BasicVar::MayMustT x)
{
  switch(x) {
  case BasicVar::Var_MAY:  return "MAY";
  case BasicVar::Var_MUST: return "MUST";
  }
}

}  // end namespace RAnnot
