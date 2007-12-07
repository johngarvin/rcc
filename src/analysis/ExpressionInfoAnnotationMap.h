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

// File: ExpressionInfoAnnotationMap.h
//
// Maps expression SEXPs to ExpressionInfo annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef EXPRESSION_INFO_ANNOTATION_MAP_H
#define EXPRESSION_INFO_ANNOTATION_MAP_H

// Set of ExpressionInfo annotations. Note that this AnnotationMap is
// computed built piecewise instead of all at once. Thus it uses a
// different interface than DefaultAnnotationMap. Note: owns values in
// map, so they must be deleted in the destructor

#include <map>

#include <analysis/AnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class ExpressionInfoAnnotationMap : public AnnotationMap {
public:
  // deconstructor
  virtual ~ExpressionInfoAnnotationMap();

  // singleton
  static ExpressionInfoAnnotationMap * get_instance();

  // getting the handle causes this map to be created and registered
  static PropertyHndlT handle();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k); // TODO: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed() const;

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

private:
  // singleton: only this class is allowed to instantiate
  ExpressionInfoAnnotationMap();

  void compute(const MyKeyT & k);

  std::map<MyKeyT, MyMappedT> m_map;

  // static members and methods for singleton
  static ExpressionInfoAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
};

}

#endif
