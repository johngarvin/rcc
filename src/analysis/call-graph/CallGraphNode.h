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

// File: CallGraphNode.h
//
// Abstract class representing a node of a call graph
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef CALL_GRAPH_NODE_H
#define CALL_GRAPH_NODE_H

#include <ostream>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/call-graph/RccCallGraphAnnotationMap.h>

namespace RAnnot {

class RccCallGraphAnnotation;

/// Abstract class representing a node of a call graph
class CallGraphNode {
public:
  explicit CallGraphNode();
  virtual ~CallGraphNode();

  virtual const OA::IRHandle get_handle() const = 0;

  virtual void dump(std::ostream & os) const = 0;
  virtual void dump_string(std::ostream & os) const = 0;

  /// compute the outgoing edges and add them to the graph
  virtual void compute(RccCallGraphAnnotationMap::NodeListT & worklist,
		       RccCallGraphAnnotationMap::NodeSetT & visited) const = 0;

  /// perform this node's part in traversing the graph to build bindings of a call site
  virtual void get_call_bindings(RccCallGraphAnnotationMap::NodeListT & worklist,
				 RccCallGraphAnnotationMap::NodeSetT & visited,
				 RccCallGraphAnnotation * ann) const = 0;

  /// get the unique node ID
  int get_id() const;
private:
  // prevent copying
  CallGraphNode & operator=(const CallGraphNode &);
  explicit CallGraphNode(const CallGraphNode &);

  int m_id;
  static int m_max_id;
};

} // end namespace RAnnot

#endif
