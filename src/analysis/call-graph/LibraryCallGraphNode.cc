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

// File:
//
// A call graph node representing an R internal value such as a
// library function.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/DumpMacros.h>
#include <support/RccError.h>

#include <analysis/HandleInterface.h>

#include <analysis/call-graph/CallGraphAnnotation.h>

#include "LibraryCallGraphNode.h"

namespace RAnnot {

  LibraryCallGraphNode::LibraryCallGraphNode(const SEXP name, const SEXP value)
    : m_name(name), m_value(value)
  {}

  LibraryCallGraphNode::~LibraryCallGraphNode()
  {}

  const OA::IRHandle LibraryCallGraphNode::get_handle() const {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(m_name));
  }
  const SEXP LibraryCallGraphNode::get_name() const {
    return m_name;
  }
  const SEXP LibraryCallGraphNode::get_value() const {
    return m_value;
  }

  void LibraryCallGraphNode::
  compute(CallGraphAnnotationMap::NodeListT & worklist,
	  CallGraphAnnotationMap::NodeSetT & visited) const
  {
    rcc_error("compute: Library call graph node incorrectly appeared in worklist");
  }

  void LibraryCallGraphNode::
  get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
		    CallGraphAnnotationMap::NodeSetT & visited,
		    CallGraphAnnotation * ann) const
  {
    ann->insert_node(this);
  }

  void LibraryCallGraphNode::dump(std::ostream & os) const {
    beginObjDump(os, LibraryCallGraphNode);
    dumpVar(os, get_id());
    dumpSEXP(os, m_name);
    endObjDump(os, LibraryCallGraphNode);
  }

  void LibraryCallGraphNode::dump_string(std::ostream & os) const {
    std::string first_name = CHAR(PRINTNAME(m_name));
    beginObjDump(os, LibraryCallGraphNode);
    dumpVar(os, get_id());
    dumpString(os, first_name);
    endObjDump(os, LibraryCallGraphNode);
  }

} // end namespace RAnnot
