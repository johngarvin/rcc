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

// File: CallByValueInfoAnnotationMap.h
//
// Maps each call site to CallByValueInfo with eager/lazy info for actual arguments.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef CALL_BY_VALUE_INFO_ANNOTATION_MAP_H
#define CALL_BY_VALUE_INFO_ANNOTATION_MAP_H

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class CallByValueInfoAnnotationMap : public DefaultAnnotationMap {
public:
  virtual ~CallByValueInfoAnnotationMap();

  /// instance for singleton pattern
  static CallByValueInfoAnnotationMap * instance();

  /// getting the handle causes this map to be created and registered
  static PropertyHndlT handle();

private:
  /// private constructor for singleton pattern
  CallByValueInfoAnnotationMap();

  void compute();

  static CallByValueInfoAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
  static void create();
};

} // end namespace RAnnot

#endif
