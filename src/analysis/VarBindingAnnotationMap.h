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

// File: VarBindingAnnotationMap.h
//
// Maps each variable to a VarBinding that describes its binding
// scopes. Owns the values in its map and must delete them in destructor.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VAR_BINDING_ANNOTATION_MAP_H
#define VAR_BINDING_ANNOTATION_MAP_H

#include <map>

#include <OpenAnalysis/Utils/Iterator.hpp>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class VarBindingAnnotationMap : public DefaultAnnotationMap
{
public:
  // deconstructor
  virtual ~VarBindingAnnotationMap();

  // singleton pattern
  static VarBindingAnnotationMap * get_instance();
  static PropertyHndlT handle();

private:
  // private constructor for singleton pattern
  explicit VarBindingAnnotationMap();

  void compute();
  void create_var_bindings();
  void populate_symbol_tables();

  static void create();

private:
  static VarBindingAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
};

}

#endif
