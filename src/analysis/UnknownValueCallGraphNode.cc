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
#include <analysis/CallGraphAnnotation.h>
#include <analysis/CallGraphInfo.h>
#include <analysis/Utils.h>

#include "UnknownValueCallGraphNode.h"

namespace RAnnot {

  UnknownValueCallGraphNode::UnknownValueCallGraphNode()
  {}

  UnknownValueCallGraphNode::~UnknownValueCallGraphNode()
  {}

  const OA::IRHandle UnknownValueCallGraphNode::get_handle() const
  {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(get_instance()));
  }

  void UnknownValueCallGraphNode::
  compute(CallGraphAnnotationMap::NodeListT & worklist,
	  CallGraphAnnotationMap::NodeSetT & visited) const
  {
    rcc_error("Internal error: compute: UnknownValueCallGraphNode found on worklist");
  }

  void UnknownValueCallGraphNode::
  get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
		    CallGraphAnnotationMap::NodeSetT & visited,
		    CallGraphAnnotation * ann) const
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
  get_instance() {
    if (m_instance == 0) {
      m_instance = new UnknownValueCallGraphNode();
    }
    return m_instance;
  }
  
  UnknownValueCallGraphNode * UnknownValueCallGraphNode::
  m_instance = 0;

} // end RAnnot namespace
