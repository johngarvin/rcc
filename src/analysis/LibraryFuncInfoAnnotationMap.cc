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
// File: LibraryFuncInfoAnnotationMap.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/Assertion.h>
#include <analysis/HandleInterface.h>
#include <analysis/LibraryFuncInfo.h>
#include <analysis/SexpTraversal.h>
#include <analysis/Utils.h>

#include "LibraryFuncInfoAnnotationMap.h"

namespace RAnnot {

  // ----- constructor/deconstructor -----

LibraryFuncInfoAnnotationMap::LibraryFuncInfoAnnotationMap() {
}

LibraryFuncInfoAnnotationMap::~LibraryFuncInfoAnnotationMap() {
}

// ----- singleton pattern -----

LibraryFuncInfoAnnotationMap * LibraryFuncInfoAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT LibraryFuncInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

// ----- computation -----

void LibraryFuncInfoAnnotationMap::compute() {
  FuncInfo * fi;
  std::list<SEXP> libs;
  FOR_EACH_PROC(fi) {
    PROC_FOR_EACH_CALL_SITE(fi, csi) {
      SEXP lhs = call_lhs(CAR(*csi));
      if (is_var(lhs) && is_library(lhs) && is_closure(library_value(lhs))) {
	LibraryFuncInfo * lfi = new LibraryFuncInfo(lhs, library_value(lhs));
	get_map()[library_value(lhs)] = lfi;
      }
    }
  }
}


LibraryFuncInfoAnnotationMap * LibraryFuncInfoAnnotationMap::s_instance = 0;
PropertyHndlT LibraryFuncInfoAnnotationMap::s_handle = "LibraryFuncInfo";

void LibraryFuncInfoAnnotationMap::create() {
  s_instance = new LibraryFuncInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}


} // namespace RAnnot
