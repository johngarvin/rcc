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

// File: Var.cc
//
// Annotation for a variable reference (includes uses and defs)
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/PropertyHndl.h>
#include <analysis/VarAnnotationMap.h>

#include "Var.h"

namespace RAnnot {

typedef Var::UseDefT UseDefT;
typedef Var::MayMustT MayMustT;

//****************************************************************************
// Var
//****************************************************************************

Var::Var(SEXP sexp, UseDefT udt, MayMustT mmt, Locality::LocalityType lt)
  : m_sexp(sexp),
    m_use_def_type(udt),
    m_may_must_type(mmt),
    m_scope_type(lt),
    m_first_on_some_path(false)
{
}


Var::~Var()
{
}

UseDefT Var::get_use_def_type() const
{
  return m_use_def_type;
}

// may/must type
MayMustT Var::get_may_must_type() const 
{
  return m_may_must_type;
}

// scope type
Locality::LocalityType Var::get_scope_type() const
{
  return m_scope_type;
}

void Var::set_scope_type(Locality::LocalityType x)
{
  m_scope_type = x;
}

// Mention (cons cell that contains the name)
SEXP Var::get_mention_c() const
{
  return m_sexp;
}

bool Var::is_first_on_some_path() const
{
  return m_first_on_some_path;
}

void Var::set_first_on_some_path(bool x)
{
  m_first_on_some_path = x;
}


std::ostream & Var::dump(std::ostream & os) const
{
  beginObjDump(os, Var);
  endObjDump(os, Var);
}

PropertyHndlT Var::handle() {
  return VarAnnotationMap::handle();
}

#if 0 
R_VarRef * Var::accept(VarVisitor<R_VarRef *> * v)
{
  VarVisitor<void *> * adapter = new VarVisitorAdapter<R_VarRef *>(v);
  return reinterpret_cast<R_VarRef *>(v_accept(adapter));
}

/// generic accept method: call the generalized, virtual version
template<class R> R Var::accept(VarVisitor<R> * v)
{
  VarVisitor<void *> adapter = new VarVisitorAdapter<R>(v);
  return dynamic_cast<R>(v_accept(adapter));
}
#endif

const std::string type_name(const Var::UseDefT x)
{
  switch(x) {
  case Var::Var_USE: return "USE";
  case Var::Var_DEF: return "DEF";
  }
}

const std::string type_name(const Var::MayMustT x)
{
  switch(x) {
  case Var::Var_MAY:  return "MAY";
  case Var::Var_MUST: return "MUST";
  }
}

}  // end namespace RAnnot
