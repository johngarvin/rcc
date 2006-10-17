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

// File: VarInfo.cc
//
// Location info about a variable.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <include/R/R_RInternals.h>

#include <support/DumpMacros.h>

#include <analysis/DefVar.h>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include "VarInfo.h"

namespace RAnnot {

//****************************************************************************
// VarInfo
//****************************************************************************

VarInfo::VarInfo()
  : m_c_location("")
{
}


VarInfo::~VarInfo()
{
}

std::string VarInfo::get_location(SubexpBuffer * sb) {
  if (m_c_location.empty()) {
    m_c_location = sb->new_location();
  }
  return m_c_location;
}


std::ostream&
VarInfo::dump(std::ostream& os) const
{
  beginObjDump(os, VarInfo);
  const_iterator it;
  for(it = beginDefs(); it != endDefs(); ++it) {
    DefVar * def = *it;
    SEXP name = def->getName();
    SEXP rhs = CAR(def->getRhs_c());

    os << "(";
    dumpSEXP(os, name);
    os << " ---> ";
    dumpSEXP(os, rhs);
    os << ")" << std::endl;
  }
  endObjDump(os, VarInfo);
}

}
