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

// File: VarAnnotationMap.h
//
// Set of Var annotations, representing basic variable information,
// including the methods for computing the information. Maps a cons
// cell containing a SYMSXP to a Var annotation. Does NOT own the
// values (Vars) in the map.
//
// TODO: This should
// probably be split up into the different analyses at some point.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VAR_ANNOTATION_MAP_H
#define VAR_ANNOTATION_MAP_H

#include <map>

#include <OpenAnalysis/CFG/CFGInterface.hpp>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

// ----- forward declarations -----

class R_IRInterface;

namespace RAnnot {

class FuncInfo;

class VarAnnotationMap : public DefaultAnnotationMap {
public:
  // destructor
  virtual ~VarAnnotationMap();

  // singleton
  static VarAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

private:
  // private constructor for singleton pattern
  explicit VarAnnotationMap();

  void compute();
  
public:
  void compute_proc(RAnnot::FuncInfo * fi);

private:
  void compute_proc_syntactic_info(RAnnot::FuncInfo * fi);
  void compute_proc_locality_info(RAnnot::FuncInfo * fi);
  void compute_locality_info(OA::OA_ptr<R_IRInterface> interface,
			     OA::ProcHandle proc,
			     OA::OA_ptr<OA::CFG::CFGInterface> cfg);

  static VarAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
  static void create();
};

}

#endif
