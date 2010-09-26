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

#include <assert.h>

#include <include/R/R_RInternals.h>

#include <support/DumpMacros.h>

#include <analysis/Var.h>
#include <analysis/LexicalScope.h>

#include <support/RccError.h>

#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include "VarInfo.h"

namespace RAnnot {

// typedefs for readability

typedef VarInfo::DefIterator DefIterator;
typedef VarInfo::ConstDefIterator ConstDefIterator;
typedef VarInfo::DefSizeT DefSizeT;
typedef VarInfo::UseIterator UseIterator;
typedef VarInfo::ConstUseIterator ConstUseIterator;
typedef VarInfo::UseSizeT UseSizeT;

VarInfo::VarInfo(const SEXP name, const LexicalScope * const scope)
  : m_name(name), m_scope(scope), m_c_location(""), m_param(false)
{
}

VarInfo::VarInfo(const SEXP name)
  : m_name(name), m_scope(), m_c_location(""), m_param(false)
{
}

VarInfo::VarInfo()
  : m_name(), m_scope(), m_c_location(""), m_param(false)
{
}

VarInfo::~VarInfo()
{
}

VarInfo * VarInfo::clone() {
  return new VarInfo(*this);
}

// defs iterators:

DefIterator VarInfo::begin_defs() {
  return m_defs.begin();
}

ConstDefIterator VarInfo::begin_defs() const {
  return m_defs.begin();
}

DefIterator VarInfo::end_defs() {
  return m_defs.end();
}

ConstDefIterator VarInfo::end_defs() const {
  return m_defs.end();
}
  
// defs capacity:
DefSizeT VarInfo::size_defs() const {
  return m_defs.size();
}
  
void VarInfo::erase_defs(DefIterator position) {
  m_defs.erase(position);
}

void VarInfo::erase_defs(DefIterator first, DefIterator last) {
  m_defs.erase(first, last);
}

void VarInfo::clear_defs() {
  m_defs.clear();
}

// uses iterators:

UseIterator VarInfo::begin_uses() {
  return m_uses.begin();
}

ConstUseIterator VarInfo::begin_uses() const {
  return m_uses.begin();
}

UseIterator VarInfo::end_uses() {
  return m_uses.end();
}

ConstUseIterator VarInfo::end_uses() const {
  return m_uses.end();
}
  
// uses capacity:
UseSizeT VarInfo::size_uses() const {
  return m_uses.size();
}
  
void VarInfo::erase_uses(UseIterator position) {
  m_uses.erase(position);
}

void VarInfo::erase_uses(UseIterator first, UseIterator last) {
  m_uses.erase(first, last);
}

void VarInfo::clear_uses() {
  m_uses.clear();
}

// modifiers:
void VarInfo::insert_var(const Var * x) {
  if (DefT def = dynamic_cast<DefT>(x)) {
    m_defs.push_back(def);
    if (def->get_source_type() == DefVar::DefVar_FORMAL) {
      m_param = true;
    }
  } else if (UseT use = dynamic_cast<UseT>(x)) {
    m_uses.push_back(use);
  } else {
    assert(0);
  }
}

bool VarInfo::is_param() {
  return m_param;
}

bool VarInfo::is_internal() {
  return (m_scope == InternalLexicalScope::instance());
}

VarInfo::DefT VarInfo::single_def_if_exists() {
  if (size_defs() == 1) {
    return *(m_defs.begin());
  } else {
    return 0;
  }
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

std::ostream & VarInfo::dump(std::ostream & os) const
{
  beginObjDump(os, VarInfo);
  dumpSEXP(os, m_name);
  if (has_scope()) {
    dumpObj(os, m_scope);
  } else {
    os << "<ambiguous: no scope specified>" << std::endl;
  }
  ConstDefIterator it;
  for(it = begin_defs(); it != end_defs(); ++it) {
    const DefVar * def = *it;
    def->dump(os);
  }
  endObjDump(os, VarInfo);
}

}
