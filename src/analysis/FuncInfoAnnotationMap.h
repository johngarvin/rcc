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
//
// File: FuncInfoAnnotationMap.h
//
// Set of FuncInfo annotations representing function information,
// including the methods for computing the information. We use a
// special destructor for the FuncInfos uses as values in the map;
// FuncInfos use the NonUniformDegreeTree library, which has its own
// mechanism for deletion.
//
// TODO: This should be split up into the different analyses at some
// point.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef FUNC_INFO_ANNOTATION_MAP_H
#define FUNC_INFO_ANNOTATION_MAP_H

#include <map>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class FuncInfo;

class FuncInfoAnnotationMap : public DefaultAnnotationMap {
public:
  // deconstructor
  virtual ~FuncInfoAnnotationMap();
  
  // singleton
  static FuncInfoAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  FuncInfo * get_scope_tree_root();
private:
  /// private constructor for singleton pattern
  explicit FuncInfoAnnotationMap();

  static void create();

  /// traverse the program, create a FuncInfo for each function
  /// definition with the whole program as the root
  void compute();

  /// recursively build FuncInfo around basic with the given parent
  void build_tree(FuncInfo * parent, BasicFuncInfo * basic);

  /// Get FuncInfos for libraries called in the program
  void collect_libraries();

  static FuncInfoAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
  FuncInfo * m_root;
};

}

#endif
