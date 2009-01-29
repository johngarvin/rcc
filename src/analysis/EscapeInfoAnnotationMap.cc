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

// File: EscapeInfoAnnotationMap.cc
//
// Maps expression SEXPs to information from escape analysis.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/Debug.h>

#include <analysis/AnalysisResults.h>
#include <analysis/PropertyHndl.h>

#include "EscapeInfoAnnotationMap.h"

namespace RAnnot {

// ----- type definitions for readability -----

typedef EscapeInfoAnnotationMap::MyKeyT MyKeyT;
typedef EscapeInfoAnnotationMap::MyMappedT MyMappedT;

// ----- constructor/destructor ----- 

EscapeInfoAnnotationMap::EscapeInfoAnnotationMap()
{
  // RCC_DEBUG("RCC_EscapeInfoAnnotationMap", debug);
}

EscapeInfoAnnotationMap::~EscapeInfoAnnotationMap() {
  // owns EscapeInfo annotations, so delete them in deconstructor
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = get_map().begin(); iter != get_map().end(); ++iter) {
    delete(iter->second);
  }
}


// ----- computation -----

void EscapeInfoAnnotationMap::compute() {
}


// ----- singleton pattern -----

EscapeInfoAnnotationMap * EscapeInfoAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT EscapeInfoAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void EscapeInfoAnnotationMap::create() {
  m_instance = new EscapeInfoAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

EscapeInfoAnnotationMap * EscapeInfoAnnotationMap::m_instance = 0;
PropertyHndlT EscapeInfoAnnotationMap::m_handle = "EscapeInfo";


} // end namespace RAnnot
