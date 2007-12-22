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

#include <analysis/Var.h>
#include <analysis/DefVar.h>
#include <analysis/LexicalScope.h>

#include <support/RccError.h>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include "VarInfo.h"

namespace RAnnot {

  // typedefs for readability

typedef VarInfo::iterator iterator;
typedef VarInfo::const_iterator const_iterator;
typedef VarInfo::size_type size_type;

VarInfo::VarInfo(const SEXP name, const LexicalScope * const scope)
  : m_name(name), m_scope(scope), m_c_location(""), m_param(false)
{
}

VarInfo::VarInfo(const SEXP name)
  : m_name(name), m_scope(), m_c_location(""), m_param(false)
{
}

VarInfo::~VarInfo()
{
}

VarInfo * VarInfo::clone() {
  return new VarInfo(*this);
}

// defs iterators:

iterator VarInfo::begin_defs() {
  return m_defs.begin();
}

const_iterator VarInfo::begin_defs() const {
  return m_defs.begin();
}

iterator VarInfo::end_defs() {
  return m_defs.end();
}

const_iterator VarInfo::end_defs() const {
  return m_defs.end();
}
  
// defs capacity:
size_type VarInfo::size_defs() const {
  return m_defs.size();
}
  
// defs modifiers:
void VarInfo::insert_def(const value_type& x) {
  m_defs.push_back(x);
  if (x->getSourceType() == DefVar::DefVar_FORMAL) {
    m_param = true;
  }
}

iterator VarInfo::insert_def(iterator position, const value_type& x) {
  return m_defs.insert(position, x);
}

void VarInfo::erase_defs(iterator position) {
  m_defs.erase(position);
}

void VarInfo::erase_defs(iterator first, iterator last) {
  m_defs.erase(first, last);
}

void VarInfo::clear_defs() {
  m_defs.clear();
}

bool VarInfo::is_param() {
  return m_param;
}

bool VarInfo::is_internal() {
  return (m_scope == InternalLexicalScope::get_instance());
}

std::string VarInfo::get_location(SubexpBuffer * sb) {
  if (m_c_location.empty()) {
    m_c_location = sb->new_location();
  }
  return m_c_location;
}

const SEXP VarInfo::get_name() const {
  return m_name;
}

const LexicalScope * const VarInfo::get_scope() const {
  if (!has_scope()) {
    rcc_error("VarInfo: tried to get lexical scope of a VarInfo in the ambiguous symbol table");
  }
  return m_scope;
}

bool VarInfo::has_scope() const {
  return (m_scope != 0);
}

std::ostream&
VarInfo::dump(std::ostream& os) const
{
  beginObjDump(os, VarInfo);
  dumpSEXP(os, m_name);
  dumpObj(os, m_scope);
  const_iterator it;
  for(it = begin_defs(); it != end_defs(); ++it) {
    DefVar * def = *it;
    def->dump(os);
  }
  endObjDump(os, VarInfo);
}

}
