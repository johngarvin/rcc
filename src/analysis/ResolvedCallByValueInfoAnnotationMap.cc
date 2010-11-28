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

// File: ResolvedCallByValueInfoAnnotationMap.cc
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisResults.h>
#include <analysis/CallByValueTraversal.h>

#include "ResolvedCallByValueInfoAnnotationMap.h"

namespace RAnnot {

// ----- destructor -----

ResolvedCallByValueInfoAnnotationMap::~ResolvedCallByValueInfoAnnotationMap() {
  delete_map_values();
}

// ----- Singleton pattern -----

ResolvedCallByValueInfoAnnotationMap * ResolvedCallByValueInfoAnnotationMap::instance() {
  if (s_instance == 0) {
    create();
  }
  return s_instance;
}

PropertyHndlT ResolvedCallByValueInfoAnnotationMap::handle() {
  if (s_instance == 0) {
    create();
  }
  return s_handle;
}

ResolvedCallByValueInfoAnnotationMap::ResolvedCallByValueInfoAnnotationMap() {
}

ResolvedCallByValueInfoAnnotationMap * ResolvedCallByValueInfoAnnotationMap::s_instance = 0;
PropertyHndlT ResolvedCallByValueInfoAnnotationMap::s_handle = "ResolvedCallByValueInfo";

void ResolvedCallByValueInfoAnnotationMap::create() {
  s_instance = new ResolvedCallByValueInfoAnnotationMap();
  analysisResults.add(s_handle, s_instance);
}

// ----- computation -----

void ResolvedCallByValueInfoAnnotationMap::compute() {
  CallByValueTraversal::instance();
}

} // end namespace RAnnot
