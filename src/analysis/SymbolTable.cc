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
// them. There's a SymbolTable for each lexical scope.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <ostream>

#include <support/DumpMacros.h>

#include <analysis/VarInfo.h>

#include "SymbolTable.h"

namespace RAnnot {

typedef SymbolTable::iterator iterator;
typedef SymbolTable::const_iterator const_iterator;
typedef SymbolTable::mapped_type mapped_type;
typedef SymbolTable::size_type size_type;

SymbolTable::SymbolTable()
{
}


SymbolTable::~SymbolTable()
{
}

SymbolTable * SymbolTable::clone()
{
  return new SymbolTable(*this);
}

iterator SymbolTable::begin()
{
  return m_vars.begin();
}

iterator SymbolTable::end()
{
  return m_vars.end();
}

const_iterator SymbolTable::begin() const
{
  return m_vars.begin();
}

const_iterator SymbolTable::end() const 
{
  return m_vars.end();
}

size_type SymbolTable::size() const
{
  return m_vars.size();
}

mapped_type & SymbolTable::operator[](const key_type & x)
{
  return m_vars[x];
}

std::pair<iterator, bool> SymbolTable::insert(const value_type & x)
{
  return m_vars.insert(x);
}

iterator SymbolTable::insert(iterator position, const value_type & x)
{
  return m_vars.insert(position, x);
}

void SymbolTable::erase(iterator position) 
{
  m_vars.erase(position);
}

size_type SymbolTable::erase(const key_type & x) 
{
  return m_vars.erase(x);
}

void SymbolTable::erase(iterator first, iterator last) 
{
  return m_vars.erase(first, last);
}

void SymbolTable::clear()
{
  m_vars.clear();
}

iterator SymbolTable::find(const key_type & x)
{
  return m_vars.find(x);
}

const_iterator SymbolTable::find(const key_type & x) const
{
  return m_vars.find(x);
}

size_type SymbolTable::count(const key_type & x) const
{
  return m_vars.count(x);
}


SymbolTable::mapped_type SymbolTable::find_or_create(const key_type & k, const LexicalScope * const scope)
{
  mapped_type value;
  iterator entry = find(k);
  if (entry == end()) {
    value = new VarInfo(k, scope);
    (*this)[k] = value;
  } else {
    value = entry->second;
  }
  return value;
}

std::ostream & SymbolTable::dump(std::ostream & os) const
{
  beginObjDump(os, SymbolTable);
  for (const_iterator it = this->begin(); it != this->end(); ++it) {
    dumpObj(os, it->second);
  }
  endObjDump(os, SymbolTable);
}

SymbolTable * SymbolTable::get_ambiguous_st() {
  if (s_ambiguous_st == 0) {
    s_ambiguous_st = new SymbolTable();
  }
  return s_ambiguous_st;
}

SymbolTable * SymbolTable::s_ambiguous_st = 0;

} // end namespace RAnnot
