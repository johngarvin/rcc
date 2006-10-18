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

// File: CallGraphEdge.h
//
// An edge between two CallGraphNodes in the call graph.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef CALL_GRAPH_EDGE_H
#define CALL_GRAPH_EDGE_H

#include <analysis/CallGraphAnnotationMap.h>
#include <analysis/CallGraphNode.h>

namespace RAnnot {

class CallGraphEdge {
public:
  explicit CallGraphEdge(const CallGraphNode * const source,
			 const CallGraphNode * const sink);
  virtual ~CallGraphEdge();

  const CallGraphNode * const get_source() const;
  const CallGraphNode * const get_sink() const;

private:
  const CallGraphNode * const m_source;
  const CallGraphNode * const m_sink;
};

} // end namespace RAnnot

#endif
