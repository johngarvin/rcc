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

// File: BasicVarAnnotationMap.h
//
// Set of BasicVar annotations, representing basic variable information,
// including the methods for computing the information. Maps a cons
// cell containing a SYMSXP to a BasicVar annotation.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef BASIC_VAR_ANNOTATION_MAP_H
#define BASIC_VAR_ANNOTATION_MAP_H

#include <map>

#include <OpenAnalysis/CFG/CFGInterface.hpp>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/PropertyHndl.h>

// ----- forward declarations -----

class R_IRInterface;

namespace RAnnot {

class BasicFuncInfo;

class BasicVarAnnotationMap : public DefaultAnnotationMap {
public:
  // destructor
  virtual ~BasicVarAnnotationMap();

  // singleton
  static BasicVarAnnotationMap * instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

private:
  // private constructor for singleton pattern
  explicit BasicVarAnnotationMap();

  void compute();
  
  static BasicVarAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
  static void create();
};

}

#endif
