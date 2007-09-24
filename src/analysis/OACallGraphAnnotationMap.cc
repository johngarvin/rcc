// -*- Mode: C++ -*-
//
// Copyright (c) 2007 Rice University
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

// File: OACallGraphAnnotationMap.cc
//
// Represents the OpenAnalysis call graph of the program.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/AnalysisResults.h>

#include "OACallGraphAnnotationMap.h"

namespace RAnnot {

// ----- type definitions for readability -----
  
typedef OACallGraphAnnotationMap::MyKeyT MyKeyT;
typedef OACallGraphAnnotationMap::MyMappedT MyMappedT;
typedef OACallGraphAnnotationMap::iterator iterator;
typedef OACallGraphAnnotationMap::const_iterator const_iterator;

// ----- constructor, destructor -----

OACallGraphAnnotationMap::OACallGraphAnnotationMap() : m_computed(false)
{
}

OACallGraphAnnotationMap::~OACallGraphAnnotationMap()
{
}

// ----- singleton pattern -----

OACallGraphAnnotationMap * OACallGraphAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}
  
PropertyHndlT OACallGraphAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}
  
void OACallGraphAnnotationMap::create() {
  m_instance = new OACallGraphAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}
  
OACallGraphAnnotationMap * OACallGraphAnnotationMap::m_instance = 0;
PropertyHndlT OACallGraphAnnotationMap::m_handle = "OACallGraph";

// ----- demand-driven analysis -----

// TODO: remove this when refactoring is done
MyMappedT & OACallGraphAnnotationMap::operator[](const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // TODO
}

/// given a call site, return the list of fundef/library nodes reachable; compute if necessary
MyMappedT OACallGraphAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // TODO
}


bool OACallGraphAnnotationMap::is_computed() {
  return m_computed;
}

void OACallGraphAnnotationMap::compute() {
}

//  ----- iterators ----- 

iterator OACallGraphAnnotationMap::begin() {  }
iterator OACallGraphAnnotationMap::end() {  }
const_iterator OACallGraphAnnotationMap::begin() const {  }
const_iterator OACallGraphAnnotationMap::end() const {  }

// ----- debugging -----
void OACallGraphAnnotationMap::dump(std::ostream & os) {
  // TODO
}

void OACallGraphAnnotationMap::dumpdot(std::ostream & os) {
  // TODO
}


} // namespace RAnnot
