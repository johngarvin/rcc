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

// File: OACallGraphAnnotation.cc
//
// Annotation for call graph information. Contains a
// ProcHandleIterator that iterates through each procedure that may be
// called.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/OACallGraphAnnotationMap.h>

#include "OACallGraphAnnotation.h"

namespace RAnnot {

OACallGraphAnnotation::OACallGraphAnnotation(const OA::OA_ptr<OA::ProcHandleIterator> iter)
  : m_iter(iter) 
{}

OACallGraphAnnotation::~OACallGraphAnnotation()
{}

OA::OA_ptr<OA::ProcHandleIterator> OACallGraphAnnotation::get_iterator() const {
  return m_iter;
}

const OA::ProcHandle OACallGraphAnnotation::get_singleton_if_exists() const {
  OA::ProcHandle ph;
  m_iter->reset();
  if (m_iter->isValid()) {
    ph = m_iter->current();
    (*m_iter)++;
    if (m_iter->isValid()) {  // >1 elements
      ph = 0;
    }
  } else {                  // 0 elements
    ph = 0;
  }
  m_iter->reset();
  return ph;
}

AnnotationBase * OACallGraphAnnotation::clone() {
  return 0;  // don't support cloning
}

PropertyHndlT OACallGraphAnnotation::handle() {
  return OACallGraphAnnotationMap::handle();
}

std::ostream & OACallGraphAnnotation::dump(std::ostream & stream) const {
  stream << "Nodes:" << std::endl;
  for(m_iter->reset(); m_iter->isValid(); (*m_iter)++) {
    stream << "node" << std::endl;
  }
}

} // end namespace RAnnot
