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

// File: FormalArgInfoAnnotationMap.h
//
// Maps formal arguments of a function to annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef FORMAL_ARG_INFO_ANNOTATION_MAP_H
#define FORMAL_ARG_INFO_ANNOTATION_MAP_H

// Set of FormalArgInfo annotations. Owns values in map, so they must
// be deleted in the destructor.

#include <map>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class FormalArgInfoAnnotationMap : public DefaultAnnotationMap {
public:
  // constructor/deconstructor
  explicit FormalArgInfoAnnotationMap();
  virtual ~FormalArgInfoAnnotationMap();

  // singleton
  static FormalArgInfoAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

private:
  void compute();

private:
  static FormalArgInfoAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
};

}

#endif
