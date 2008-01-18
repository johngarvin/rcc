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

// File: DefaultAnnotationMap.h
//
// Common implementation details for most annotation maps.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "DefaultAnnotationMap.h"

#include <support/RccError.h>

namespace RAnnot {

// ----- type definitions for readability -----

typedef DefaultAnnotationMap::MyKeyT MyKeyT;
typedef DefaultAnnotationMap::MyMappedT MyMappedT;
typedef DefaultAnnotationMap::iterator iterator;
typedef DefaultAnnotationMap::const_iterator const_iterator;

DefaultAnnotationMap::DefaultAnnotationMap()
  : m_computed(false),
    m_computation_in_progress(false),
    m_map()
{}

DefaultAnnotationMap::~DefaultAnnotationMap() {}

MyMappedT & DefaultAnnotationMap::operator[](const MyKeyT & k) {
  compute_if_necessary();
  return m_map[k];  
}

MyMappedT DefaultAnnotationMap::get(const MyKeyT & k) {
  compute_if_necessary();
  
  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  if (!is_valid(k)) {
    rcc_error("DefaultAnnotationMap: possible invalid key not found in map");
  } else {
    return m_map[k];
  }  
}

bool DefaultAnnotationMap::is_valid(const MyKeyT & k) {
  compute_if_necessary();
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_map.find(k);
  return (annot != m_map.end());
}

bool DefaultAnnotationMap::is_computed() const {
  return m_computed;
}

// if we screwed up dependences between annotation maps, compute()
// might wind up calling itself in an infinite loop. Set/clear a
// "computation in progress" bit to fail gracefully in that case.
bool DefaultAnnotationMap::computation_in_progress() const {
  return m_computation_in_progress;
}

void DefaultAnnotationMap::compute_if_necessary() {
  if (is_computed()) {
    return;
  }
  if (computation_in_progress()) {
    rcc_error("Compiler bug: annotation map depends on itself");
  }
  m_computation_in_progress = true;
  compute();
  m_computed = true;
  m_computation_in_progress = false;
}

iterator DefaultAnnotationMap::begin() { return m_map.begin(); }

const_iterator DefaultAnnotationMap::begin() const { return m_map.begin(); }

iterator DefaultAnnotationMap::end() { return m_map.end(); }

const_iterator DefaultAnnotationMap::end() const { return m_map.end(); }

std::map<MyKeyT, MyMappedT> & DefaultAnnotationMap::get_map() {
  return m_map;
}

void DefaultAnnotationMap::delete_map_values() {
  std::map<MyKeyT, MyMappedT>::const_iterator iter;
  for(iter = m_map.begin(); iter != m_map.end(); ++iter) {
    delete(iter->second);
  }
}

} // end namespace RAnnot
