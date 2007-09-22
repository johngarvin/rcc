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

// File: CoordinateCallGraphNode.h
//
// Call graph node representing a coordinate (name + scope).
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef COORDINATE_CALL_GRAPH_NODE_H
#define COORDINATE_CALL_GRAPH_NODE_H

#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/LexicalScope.h>

#include <analysis/call-graph/CallGraphNode.h>

namespace RAnnot {

class CoordinateCallGraphNode : public CallGraphNode {
public:
  explicit CoordinateCallGraphNode(const SEXP name, const LexicalScope * scope);
  virtual ~CoordinateCallGraphNode();

  const OA::IRHandle get_handle() const;

  const SEXP get_name() const;
  const LexicalScope * get_scope() const;

  void compute(RccCallGraphAnnotationMap::NodeListT & worklist,
	       RccCallGraphAnnotationMap::NodeSetT & visited) const;

  void get_call_bindings(RccCallGraphAnnotationMap::NodeListT & worklist,
			 RccCallGraphAnnotationMap::NodeSetT & visited,
			 RccCallGraphAnnotation * ann) const;

  void dump(std::ostream & os) const;
  void dump_string(std::ostream & os) const;
private:
  const SEXP m_name;
  const LexicalScope * m_scope;
};

} // end namespace RAnnot

#endif
