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

// File: FormalArgInfoAnnotationMap.cc
//
// Maps formal arguments of a function to annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/BasicFuncInfo.h>
#include <analysis/BasicFuncInfoAnnotationMap.h>
#include <analysis/FormalArgInfo.h>

#include "FormalArgInfoAnnotationMap.h"

namespace RAnnot {

// type definitions for readability
typedef FormalArgInfoAnnotationMap::MyKeyT MyKeyT;
typedef FormalArgInfoAnnotationMap::MyMappedT MyMappedT;
typedef FormalArgInfoAnnotationMap::iterator iterator;
typedef FormalArgInfoAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor -----

FormalArgInfoAnnotationMap::FormalArgInfoAnnotationMap() {}

FormalArgInfoAnnotationMap::~FormalArgInfoAnnotationMap() {
  delete_map_values();
}

// ----- singleton pattern -----

FormalArgInfoAnnotationMap * FormalArgInfoAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT FormalArgInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

void FormalArgInfoAnnotationMap::create() {
  s_instance = new FormalArgInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

void FormalArgInfoAnnotationMap::compute() {
  BasicFuncInfo * bfi;
  FOR_EACH_BASIC_PROC_AND_LIB(bfi) {
    // user procedures
    for (SEXP e = bfi->get_args(); e != R_NilValue; e = CDR(e)) {
      get_map()[e] = new FormalArgInfo(e);
    }
  }
}

FormalArgInfoAnnotationMap * FormalArgInfoAnnotationMap::s_instance = 0;
PropertyHndlT FormalArgInfoAnnotationMap::s_handle = "FormalArgInfo";

}
