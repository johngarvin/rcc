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

// File: SymbolTable.cc
//
// A table mapping names (SEXPs of SYMSXP type) to VarInfos describing
// them. There's a SymbolTable for each function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/SymbolTableAnnotationMap.h>
#include <analysis/VarInfo.h>

#include "SymbolTable.h"

namespace RAnnot {

SymbolTable::SymbolTable()
{
}


SymbolTable::~SymbolTable()
{
}

PropertyHndlT SymbolTable::handle() {
  return SymbolTableAnnotationMap::handle();
}

std::ostream& SymbolTable::dump(std::ostream& os) const
{
  beginObjDump(os, SymbolTable);
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    dumpObj(os, it->second);
  }
  endObjDump(os, SymbolTable);
}

} // end namespace RAnnot
