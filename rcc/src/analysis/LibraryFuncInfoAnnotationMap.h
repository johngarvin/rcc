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
// File: LibraryFuncInfoAnnotationMap.h
//
// Set of LibraryFuncInfo annotations representing function information,
// including the methods for computing the information. 
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef LIBRARY_FUNC_INFO_ANNOTATION_MAP_H
#define LIBRARY_FUNC_INFO_ANNOTATION_MAP_H

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {
  
class LibraryFuncInfoAnnotationMap : public DefaultAnnotationMap {
public:
  // deconstructor
  virtual ~LibraryFuncInfoAnnotationMap();
  
  // singleton
  static LibraryFuncInfoAnnotationMap * instance();
  
  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

private:
  /// private constructor for singleton pattern
  explicit LibraryFuncInfoAnnotationMap();
  
  /// traverse the program, create a FuncInfo for each function
  /// definition with the whole program as the root
  void compute();
  
  static void create();

  static LibraryFuncInfoAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
};
  
}

#endif // LIBRARY_FUNC_INFO_ANNOTATION_MAP_H
