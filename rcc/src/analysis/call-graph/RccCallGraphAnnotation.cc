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

// File: CallGraphAnnotation.cc
//
// Annotation for call graph information. Contains a set of
// CallGraphNodes. Returned by CallGraphAnnotation::get_call_bindings;
// it's a container for the relevant list of nodes computed by that
// method.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <analysis/call-graph/RccCallGraphAnnotationMap.h>

#include "RccCallGraphAnnotation.h"

namespace RAnnot {

  // typedef for readability
  typedef RccCallGraphAnnotation::MySetT MySetT;

  RccCallGraphAnnotation::RccCallGraphAnnotation()
  {}

  RccCallGraphAnnotation::~RccCallGraphAnnotation()
  {}

  // ----- methods to insert nodes -----

  void RccCallGraphAnnotation::insert_node(const CallGraphNode * const node) {
    m_nodes.insert(node);
  }

  // ----- iterators -----

  MySetT::const_iterator RccCallGraphAnnotation::begin() const {
    return m_nodes.begin();
  }

  MySetT::const_iterator RccCallGraphAnnotation::end() const {
    return m_nodes.end();
  }

  // ----- get_singleton_if_exists -----

  const CallGraphNode * RccCallGraphAnnotation::get_singleton_if_exists() const {
    if (begin() == end()) return 0;
    MySetT::const_iterator elt1 = begin();
    MySetT::const_iterator elt2 = begin();
    elt2++;
    if (elt2 == end()) {  // set contains one element
      return *elt1;
    } else {
      return 0;
    }
  }

  // ----- AnnotationBase -----

  AnnotationBase * RccCallGraphAnnotation::clone() {
    return 0; // don't support this
  }

  PropertyHndlT RccCallGraphAnnotation::handle() {
    return RccCallGraphAnnotationMap::handle();
  }

  std::ostream & RccCallGraphAnnotation::dump(std::ostream & stream) const {
    MySetT::const_iterator it;
    stream << "Nodes:" << std::endl;
    for (it = begin(); it != end(); ++it) {
      stream << "node" << std::endl;
    }
  }

} // end namespace RAnnot
