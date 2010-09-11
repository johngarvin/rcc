// -*- Mode: C++ -*-
//
// Copyright (c) 2009 Rice University
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

// File: ResolvedArgAnnotationMap.h
//
// Maps each call site to a description of its arguments with named
// arguments and default arguments resolved.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef RESOLVED_ARGS_ANNOTATION_MAP_H
#define RESOLVED_ARGS_ANNOTATION_MAP_H

#include <include/R/R_Internal.h>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class ResolvedArgsAnnotationMap : public DefaultAnnotationMap
{
public:
  // singleton pattern
  static ResolvedArgsAnnotationMap * instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

private:
  // singleton pattern
  explicit ResolvedArgsAnnotationMap();

  // implement pure virtual from DefaultAnnotationMap
  void compute();

  static ResolvedArgsAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
};
  
}

#endif
