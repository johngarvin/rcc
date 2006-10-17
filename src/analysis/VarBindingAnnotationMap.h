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
// scopes.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef VAR_BINDING_ANNOTATION_MAP_H
#define VAR_BINDING_ANNOTATION_MAP_H

#include <map>

#include <OpenAnalysis/Utils/Iterator.hpp>

#include <analysis/AnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class VarBindingAnnotationMap : public AnnotationMap
{
public:
  // constructor/deconstructor
  VarBindingAnnotationMap(bool ownsAnnotations = true);
  virtual ~VarBindingAnnotationMap();

  // singleton pattern
  static VarBindingAnnotationMap * get_instance();
  static PropertyHndlT handle();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k);
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

private:
  void compute();

  static void create();

private:
  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_map;

  static VarBindingAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
};

}

#endif
