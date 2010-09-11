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

// File: SpecialProcSymMap
//
// Mapping of ProcHandles to SymHandles, for special ProcHandles that
// don't have names. This includes the "procedure" representing the
// global scope, the HellProcedure representing a lack of
// interprocedural information, and all anonymous procedures.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/HandleInterface.h>
#include <analysis/VarInfo.h>

#include "SpecialProcSymMap.h"

typedef SpecialProcSymMap::MyMapT MyMapT;
typedef SpecialProcSymMap::MySetT MySetT;

SpecialProcSymMap::SpecialProcSymMap()
  : m_global_sym(HandleInterface::make_sym_h(new RAnnot::VarInfo())),
    m_hell_sym(HandleInterface::make_sym_h(new RAnnot::VarInfo()))
{
}

SpecialProcSymMap::~SpecialProcSymMap() {
}

OA::SymHandle SpecialProcSymMap::get_global() {
  return m_global_sym;
}

OA::SymHandle SpecialProcSymMap::get_hell() {
  return m_hell_sym;
}

bool SpecialProcSymMap::is_anon(OA::ProcHandle ph) {
  MyMapT::const_iterator it = m_anons.find(ph);
  return (it != m_anons.end());
}
  
bool SpecialProcSymMap::is_anon(OA::SymHandle sh) {
  MySetT::const_iterator it = m_anon_syms.find(sh);
  return (it != m_anon_syms.end());
}

OA::SymHandle SpecialProcSymMap::get_anon(OA::ProcHandle ph) {
  MyMapT::const_iterator it = m_anons.find(ph);
  if (it == m_anons.end()) {
    OA::SymHandle s = HandleInterface::make_sym_h(new RAnnot::VarInfo());
    m_anons[ph] = s;
    m_anon_syms.insert(s);
    return s;
  } else {
    return it->second;
  }
}

SpecialProcSymMap * SpecialProcSymMap::instance() {
  if (s_instance == 0) {
    s_instance = new SpecialProcSymMap();
  }
  return s_instance;
}

SpecialProcSymMap * SpecialProcSymMap::s_instance = 0;
