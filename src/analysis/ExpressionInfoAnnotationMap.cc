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

// File: ExpressionInfoAnnotationMap.cc
//
// Maps expression SEXPs to ExpressionInfo annotations.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <map>

#include <analysis/AnalysisResults.h>
#include <analysis/ExpressionInfo.h>
#include <analysis/HandleInterface.h>
#include <analysis/LocalVariableAnalysis.h>
#include <analysis/PropertySet.h>

#include "ExpressionInfoAnnotationMap.h"

using namespace HandleInterface;

namespace RAnnot {

// type definitions for readability
typedef ExpressionInfoAnnotationMap::MyKeyT MyKeyT;
typedef ExpressionInfoAnnotationMap::MyMappedT MyMappedT;
typedef ExpressionInfoAnnotationMap::iterator iterator;
typedef ExpressionInfoAnnotationMap::const_iterator const_iterator;

// ----- constructor/destructor ----- 

ExpressionInfoAnnotationMap::ExpressionInfoAnnotationMap()
  : m_map()
  {}

ExpressionInfoAnnotationMap::~ExpressionInfoAnnotationMap() {
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = m_map.begin(); iter != m_map.end(); ++iter) {
    delete(iter->second);
  }
}

// ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// TODO: delete this when fully refactored to disallow insertion from outside.
MyMappedT & ExpressionInfoAnnotationMap::operator[](const MyKeyT & k) {
  std::map<MyKeyT, MyMappedT>::iterator annot = m_map.find(k);
  if (annot == m_map.end()) {
    compute(k);
    return m_map[k];
  } else {
    return annot->second; 
  }
}

/// Perform the computation for the given expression if necessary and
/// return the requested data.
MyMappedT ExpressionInfoAnnotationMap::get(const MyKeyT & k) {
  return (*this)[k];
}
  
/// Expression info is computed piece by piece, so it's never fully computed
bool ExpressionInfoAnnotationMap::is_computed() {
  return false;
}

//  ----- iterators ----- 

iterator ExpressionInfoAnnotationMap::begin() { return m_map.begin(); }
const_iterator ExpressionInfoAnnotationMap::begin() const { return m_map.begin(); }
iterator  ExpressionInfoAnnotationMap::end() { return m_map.end(); }
const_iterator ExpressionInfoAnnotationMap::end() const { return m_map.end(); }

// ----- computation -----

void ExpressionInfoAnnotationMap::compute(const MyKeyT & k) {
  ExpressionInfo * annot = new ExpressionInfo();
  annot->setDefn(k);
  LocalVariableAnalysis lva(k);
  lva.perform_analysis();
  LocalVariableAnalysis::const_var_iterator v;
  for(v = lva.begin_vars(); v != lva.end_vars(); ++v) {
    annot->insert_var(*v);
  }
  LocalVariableAnalysis::const_call_site_iterator cs;
  for(cs = lva.begin_call_sites(); cs != lva.end_call_sites(); ++cs) {
    annot->insert_call_site(*cs);
  }
  
  m_map[k] = annot;
}

// ----- singleton pattern -----

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}

PropertyHndlT ExpressionInfoAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}

// Create the singleton instance and register the map in PropertySet
// for getProperty
void ExpressionInfoAnnotationMap::create() {
  m_instance = new ExpressionInfoAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}

ExpressionInfoAnnotationMap * ExpressionInfoAnnotationMap::m_instance = 0;
PropertyHndlT ExpressionInfoAnnotationMap::m_handle = "ExpressionInfo";

}
