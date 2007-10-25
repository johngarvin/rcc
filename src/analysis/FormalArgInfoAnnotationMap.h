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

// Set of FormalArgInfo annotations.

#include <map>

#include <analysis/AnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class FormalArgInfoAnnotationMap : public AnnotationMap {
public:
  // constructor/deconstructor
  FormalArgInfoAnnotationMap();
  virtual ~FormalArgInfoAnnotationMap();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k); // TODO: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed() const;

  // singleton
  static FormalArgInfoAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

private:
  void compute();

private:
  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;
  
  static FormalArgInfoAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
};

}

#endif
