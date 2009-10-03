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

// File: UnknownValueCallGraphNode.h
//
// Call graph node representing an unknown value.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef UNKNOWN_VALUE_CALL_GRAPH_NODE_H
#define UNKNOWN_VALUE_CALL_GRAPH_NODE_H

#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/call-graph/CallGraphNode.h>

namespace RAnnot {

/// UnknownValueCallGraphNode represents an unknown function value in
/// the call graph. Incoming edges come from Coordinate nodes; no
/// edges go out. This class is a singleton because there's no reason
/// to distinguish different instances.
class UnknownValueCallGraphNode : public CallGraphNode {
public:
  virtual ~UnknownValueCallGraphNode();

  const OA::IRHandle get_handle() const;

  void compute(RccCallGraphAnnotationMap::NodeListT & worklist,
	       RccCallGraphAnnotationMap::NodeSetT & visited) const;

  void get_call_bindings(RccCallGraphAnnotationMap::NodeListT & worklist,
			 RccCallGraphAnnotationMap::NodeSetT & visited,
			 RccCallGraphAnnotation * ann) const;

  void dump(std::ostream & os) const;
  void dump_string(std::ostream & os) const;

  static UnknownValueCallGraphNode * get_instance();

private:
  explicit UnknownValueCallGraphNode();
  static UnknownValueCallGraphNode * s_instance;
};

} // end namespace RAnnot

#endif
