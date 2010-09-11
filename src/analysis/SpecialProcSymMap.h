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

#ifndef SPECIAL_PROC_SYM_MAP_H
#define SPECIAL_PROC_SYM_MAP_H

#include <map>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

class SpecialProcSymMap {
public:

  // ----- typedefs -----

  typedef std::map<OA::ProcHandle, OA::SymHandle> MyMapT;
  typedef std::set<OA::SymHandle>                 MySetT;
  
  // private constructor for singleton pattern

private:
  explicit SpecialProcSymMap();

  // ----- destructor -----

public:
  virtual ~SpecialProcSymMap();
  
  bool is_anon(OA::ProcHandle ph);
  bool is_anon(OA::SymHandle sh);
  OA::SymHandle get_anon(OA::ProcHandle ph);
  OA::SymHandle get_global();
  OA::SymHandle get_hell();

  // ----- singleton pattern -----

  static SpecialProcSymMap * instance();

private:
  MyMapT m_anons;
  MySetT m_anon_syms;
  OA::SymHandle m_global_sym;
  OA::SymHandle m_hell_sym;

  static SpecialProcSymMap * s_instance;
};

#endif
