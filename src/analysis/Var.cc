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

//****************************************************************************
// Var
//****************************************************************************

Var::Var()
{
}


Var::~Var()
{
}


std::ostream&
Var::dump(std::ostream& os) const
{
  beginObjDump(os, Var);
  endObjDump(os, Var);
}

PropertyHndlT
Var::handle() {
  return VarAnnotationMap::handle();
}

const std::string typeName(const Var::MayMustT x)
{
  switch(x) {
  case Var::Var_MAY:  return "MAY";
  case Var::Var_MUST: return "MUST";
  }
}

}
