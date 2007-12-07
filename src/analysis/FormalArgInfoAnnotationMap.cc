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

FormalArgInfoAnnotationMap * FormalArgInfoAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT FormalArgInfoAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

void FormalArgInfoAnnotationMap::create() {
  m_instance = new FormalArgInfoAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

// TODO: implement this (move functionality that lives in FuncInfo here)
void FormalArgInfoAnnotationMap::compute() {
}

FormalArgInfoAnnotationMap * FormalArgInfoAnnotationMap::m_instance = 0;
PropertyHndlT FormalArgInfoAnnotationMap::m_handle = "FormalArgInfo";

}
