// -*- Mode: C++ -*-
//
// Copyright (c) 2008 Rice University
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

// File: SymbolTableFacade.h
//
// Facade pattern; useful methods for dealing with SymbolTables.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef SYMBOL_TABLE_FACADE_H
#define SYMBOL_TABLE_FACADE_H

#include <analysis/FuncInfo.h>
#include <analysis/SymbolTable.h>
#include <analysis/Var.h>
#include <analysis/VarInfo.h>

class SymbolTableFacade {
private:
  explicit SymbolTableFacade();
public:
  virtual ~SymbolTableFacade();

public:
  // given a mention, find the symbol table entry
  RAnnot::VarInfo * find_entry(const SEXP sexp) const;
  
  static SymbolTableFacade * instance();

private:
  static SymbolTableFacade * s_instance;
};

#endif
