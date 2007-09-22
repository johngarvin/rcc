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

// File: FundefCallGraphNode.cc
//
// Call graph node representing a fundef (function definition).
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/DumpMacros.h>

#include <analysis/AnalysisResults.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>

#include <analysis/call-graph/RccCallGraphAnnotation.h>
#include <analysis/call-graph/CallSiteCallGraphNode.h>

#include "FundefCallGraphNode.h"

using namespace HandleInterface;

namespace RAnnot {

  FundefCallGraphNode::FundefCallGraphNode(const SEXP fundef)
    : m_fundef(fundef)
  {}

  FundefCallGraphNode::~FundefCallGraphNode()
  {}

  const OA::IRHandle FundefCallGraphNode::get_handle() const {
    return make_proc_h(m_fundef);
  }

  const SEXP FundefCallGraphNode::get_sexp() const {
    return m_fundef;
  }

  void FundefCallGraphNode::
  compute(RccCallGraphAnnotationMap::NodeListT & worklist,
	  RccCallGraphAnnotationMap::NodeSetT & visited) const
  {
    RccCallGraphAnnotationMap * const cg = RccCallGraphAnnotationMap::get_instance();
    FuncInfo * fi = getProperty(FuncInfo, m_fundef);

    // Keep track of call sites attached to this node to avoid
    // redundancy. This is a temporary workaround for a bug in which
    // the CFG has duplicate nodes. TODO: fix
    RccCallGraphAnnotationMap::NodeSetT visited_cs;

    const CallSiteCallGraphNode * csnode;
    FuncInfo::const_call_site_iterator csi;
    for(csi = fi->begin_call_sites(); csi != fi->end_call_sites(); ++csi) {
      SEXP cs = *csi;
      csnode = cg->make_call_site_node(cs);
      if (visited_cs.find(csnode) == visited_cs.end()) {
	visited_cs.insert(csnode);
	cg->add_edge(this, csnode);
	if (visited.find(csnode) == visited.end()) {
	  worklist.push_back(csnode);
	}
      }
    }
  }

  void FundefCallGraphNode::
  get_call_bindings(RccCallGraphAnnotationMap::NodeListT & worklist,
		    RccCallGraphAnnotationMap::NodeSetT & visited,
		    RccCallGraphAnnotation * ann) const
  {
    ann->insert_node(this);
  }

  void FundefCallGraphNode::dump(std::ostream & os) const {
    FuncInfo * fi = getProperty(FuncInfo, m_fundef);
    SEXP first_name = CAR(fi->get_first_name_c());

    beginObjDump(os, FundefCallGraphNode);
    dumpVar(os, get_id());
    dumpSEXP(os, first_name);
    endObjDump(os, FundefCallGraphNode);
  }

  void FundefCallGraphNode::dump_string(std::ostream & os) const {
    FuncInfo * fi = getProperty(FuncInfo, m_fundef);
    std::string first_name = CHAR(PRINTNAME(CAR(fi->get_first_name_c())));
    beginObjDump(os, FundefCallGraphNode);
    dumpVar(os, get_id());
    dumpString(os, first_name);
    endObjDump(os, FundefCallGraphNode);
  }

} // end namespace RAnnot
