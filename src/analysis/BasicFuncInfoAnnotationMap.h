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
// File: BasicFuncInfoAnnotationMap.h
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

#ifndef BASIC_FUNC_INFO_ANNOTATION_MAP_H
#define BASIC_FUNC_INFO_ANNOTATION_MAP_H

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

#define FOR_EACH_BASIC_PROC(bfi) \
  for (RAnnot::BasicFuncInfoIterator bfii = BasicFuncInfoIterator(BasicFuncInfoAnnotationMap::get_instance()->get_scope_tree_root()); \
       bfii.IsValid() && ((bfi) = bfii.Current()) != 0;			\
       ++bfii, (bfi) = bfii.Current())

#define FOR_EACH_BASIC_PROC_AND_LIB(bfi) \
  for (RAnnot::BasicFuncInfoAnnotationMap::const_iterator bfii = BasicFuncInfoAnnotationMap::get_instance()->begin(); \
       bfii != BasicFuncInfoAnnotationMap::get_instance()->end() && ((bfi) = dynamic_cast<BasicFuncInfo *>(bfii->second)) != 0; \
       ++bfii)

namespace RAnnot {
  
class BasicFuncInfoAnnotationMap : public DefaultAnnotationMap {
public:
  // deconstructor
  virtual ~BasicFuncInfoAnnotationMap();
  
  // singleton
  static BasicFuncInfoAnnotationMap * get_instance();
  
  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // BasicFuncInfo that represents the whole program
  BasicFuncInfo * get_scope_tree_root();
  
private:
  /// private constructor for singleton pattern
  explicit BasicFuncInfoAnnotationMap();
  
  /// traverse the program, create a FuncInfo for each function
  /// definition with the whole program as the root
  void compute();
  
  /// Build the scope tree with the given SEXP as the root, using the
  /// recursive version. Precondition: root must be an assignment
  /// where the right side is a function.
  void build_scope_tree(SEXP root);
  
  /// Recursively traverse e to build the scope tree. 'parent' is a
  /// pointer to the parent lexical scope.
  void build_scope_tree_rec(SEXP e, BasicFuncInfo * parent);
  
  /// Get FuncInfos for libraries called in the program
  void collect_libraries();
  
  static void create();

  static BasicFuncInfoAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
  BasicFuncInfo * m_root;
};
  
}

#endif // BASIC_FUNC_INFO_ANNOTATION_MAP_H
