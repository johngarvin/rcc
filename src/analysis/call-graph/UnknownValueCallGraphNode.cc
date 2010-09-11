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

#include <support/DumpMacros.h>
#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Utils.h>

#include <analysis/call-graph/RccCallGraphAnnotation.h>
#include <analysis/call-graph/CallGraphInfo.h>

#include "UnknownValueCallGraphNode.h"

namespace RAnnot {

  UnknownValueCallGraphNode::UnknownValueCallGraphNode()
  {}

  UnknownValueCallGraphNode::~UnknownValueCallGraphNode()
  {}

  const OA::IRHandle UnknownValueCallGraphNode::get_handle() const
  {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(instance()));
  }

  void UnknownValueCallGraphNode::
  compute(RccCallGraphAnnotationMap::NodeListT & worklist,
	  RccCallGraphAnnotationMap::NodeSetT & visited) const
  {
    rcc_error("Internal error: compute: UnknownValueCallGraphNode found on worklist");
  }

  void UnknownValueCallGraphNode::
  get_call_bindings(RccCallGraphAnnotationMap::NodeListT & worklist,
		    RccCallGraphAnnotationMap::NodeSetT & visited,
		    RccCallGraphAnnotation * ann) const
  {
    ann->insert_node(this);
  }

  void UnknownValueCallGraphNode::
  dump(std::ostream & os) const {
    beginObjDump(os, UnknownValueCallGraphNode);
    endObjDump(os, UnknownValueCallGraphNode);
  }

  void UnknownValueCallGraphNode::
  dump_string(std::ostream & os) const {
    beginObjDump(os, UnknownValueCallGraphNode);
    endObjDump(os, UnknownValueCallGraphNode);
  }
  
  UnknownValueCallGraphNode * UnknownValueCallGraphNode::
  instance() {
    if (s_instance == 0) {
      s_instance = new UnknownValueCallGraphNode();
    }
    return s_instance;
  }
  
  UnknownValueCallGraphNode * UnknownValueCallGraphNode::
  s_instance = 0;

} // end RAnnot namespace
