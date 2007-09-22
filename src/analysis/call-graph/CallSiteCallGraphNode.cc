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

// File: CallSiteCallGraphNode.cc
//
// Call graph node representing a call site. Can be a builtin or library function call.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include "CallSiteCallGraphNode.h"

#include <support/DumpMacros.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/HandleInterface.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#include <analysis/call-graph/CallGraphInfo.h>
#include <analysis/call-graph/CoordinateCallGraphNode.h>
#include <analysis/call-graph/UnknownValueCallGraphNode.h>

namespace RAnnot {

  CallSiteCallGraphNode::CallSiteCallGraphNode(const SEXP cs)
    : m_cs(cs)
  {}

  CallSiteCallGraphNode::~CallSiteCallGraphNode()
  {}

  const OA::IRHandle CallSiteCallGraphNode::get_handle() const {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(m_cs));
  }
  const SEXP CallSiteCallGraphNode::get_sexp() const {
    return m_cs;
  }

  void CallSiteCallGraphNode::
  compute(RccCallGraphAnnotationMap::NodeListT & worklist,
	  RccCallGraphAnnotationMap::NodeSetT & visited) const
  {
    RccCallGraphAnnotationMap * cg = RccCallGraphAnnotationMap::get_instance();
    if (is_var(call_lhs(m_cs))) {
      // if left side of call is a symbol
      VarBinding * binding = getProperty(VarBinding, m_cs);  // cell containing symbol
      VarBinding::const_iterator si;
      for(si = binding->begin(); si != binding->end(); ++si) {
	CoordinateCallGraphNode * node;
	node = cg->make_coordinate_node(call_lhs(m_cs), *si);
	cg->add_edge(this, node);
	if (visited.find(node) == visited.end()) {
	  worklist.push_back(node);
	}
      }
    } else {
      // LHS of call is a non-symbol expression
      cg->add_edge(this, cg->make_unknown_value_node());
    }
  }

  void CallSiteCallGraphNode::
  get_call_bindings(RccCallGraphAnnotationMap::NodeListT & worklist,
		    RccCallGraphAnnotationMap::NodeSetT & visited,
		    RccCallGraphAnnotation * ann) const
  {
    RccCallGraphAnnotationMap * cg = RccCallGraphAnnotationMap::get_instance();
    CallGraphInfo * info = cg->get_edges(this);
    // TODO: remove cast when maps are parametrized
    for (CallGraphInfo::const_iterator edge = info->begin_calls_out();
	 edge != info->end_calls_out();
	 ++edge) {
      assert(dynamic_cast<const CoordinateCallGraphNode*>((*edge)->get_sink()) != 0);
      worklist.push_back((*edge)->get_sink());
    }
  }

  void CallSiteCallGraphNode::dump(std::ostream & os) const {
    beginObjDump(os, CallSiteCallGraphNode);
    dumpVar(os, get_id());
    dumpSEXP(os, m_cs);
    endObjDump(os, CallSiteCallGraphNode);
  }

  // TODO: find a way to print the actual call site without the
  // limitations of R printing
  void CallSiteCallGraphNode::dump_string(std::ostream & os) const {
    beginObjDump(os, CallSiteCallGraphNode);
    dumpVar(os, get_id());
    endObjDump(os, CallSiteCallGraphNode);
  }

} // end namespace RAnnot
