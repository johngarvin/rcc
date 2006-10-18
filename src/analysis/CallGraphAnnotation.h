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

// File: CallGraphAnnotation.h
//
// Annotation for call graph information. Contains a set of
// CallGraphNodes. Returned by CallGraphAnnotation::get_call_bindings;
// it's a container for the relevant list of nodes computed by that
// method.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef CALL_GRAPH_ANNOTATION_H
#define CALL_GRAPH_ANNOTATION_H

#include <set>

#include <analysis/AnnotationBase.h>
#include <analysis/CallGraphEdge.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class CallGraphAnnotation : public AnnotationBase {
public:
  typedef std::set<const CallGraphNode *> MySetT;
  typedef MySetT::const_iterator const_iterator;

  explicit CallGraphAnnotation();
  virtual ~CallGraphAnnotation();

  void insert_node(const CallGraphNode * const node);

  const_iterator begin() const;
  const_iterator end() const;

  const CallGraphNode * get_singleton_if_exists() const;

  // ----- methods that implement AnnotationBase -----

  AnnotationBase * clone();

  static PropertyHndlT handle();

  // ----- debugging -----

  std::ostream & dump(std::ostream & stream) const;

private:
  MySetT m_nodes;
};

} // end namespace RAnnot

#endif
