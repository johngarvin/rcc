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

// File: CoordinateCallGraphNode.cc
//
// Call graph node representing a coordinate (name + scope).
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <support/DumpMacros.h>
#include <support/RccError.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/DefVar.h>
#include <analysis/SymbolTable.h>
#include <analysis/Utils.h>
#include <analysis/VarInfo.h>

#include <analysis/call-graph/CallGraphInfo.h>
#include <analysis/call-graph/FundefCallGraphNode.h>
#include <analysis/call-graph/LibraryCallGraphNode.h>
#include <analysis/call-graph/UnknownValueCallGraphNode.h>

#include "CoordinateCallGraphNode.h"

namespace RAnnot {

  CoordinateCallGraphNode::CoordinateCallGraphNode(const SEXP name, const LexicalScope * scope)
    : m_name(name), m_scope(scope)
  {}

  CoordinateCallGraphNode::~CoordinateCallGraphNode()
  {}

  const SEXP CoordinateCallGraphNode::get_name() const {
    return m_name;
  }

  const LexicalScope * CoordinateCallGraphNode::get_scope() const {
    return m_scope;
  }

  const OA::IRHandle CoordinateCallGraphNode::get_handle() const {
    return reinterpret_cast<OA::irhandle_t>(this);
  }

  void CoordinateCallGraphNode::
  compute(CallGraphAnnotationMap::NodeListT & worklist,
	  CallGraphAnnotationMap::NodeSetT & visited) const
  {
    CallGraphAnnotationMap * cg = CallGraphAnnotationMap::get_instance();

    if (const InternalLexicalScope * scope = dynamic_cast<const InternalLexicalScope *>(m_scope)) {
      cg->add_edge(this, cg->make_library_node(m_name, library_value(m_name)));
      // but don't add library fun to worklist
    } else if (const FundefLexicalScope * scope = dynamic_cast<const FundefLexicalScope *>(m_scope)) {
      SymbolTable * st = scope->get_symbol_table();
      SymbolTable::const_iterator it = st->find(m_name);
      if (it == st->end()) {
	// Name not found in SymbolTable: unbound variable
	rcc_error("Unbound variable " + var_name(m_name));
      } else {  // symbol is defined at least once in this scope
	VarInfo * vi = it->second;
	VarInfo::const_iterator var;
	for (var = vi->begin_defs(); var != vi->end_defs(); ++var) {
	  DefVar * def = *var;
	  if (is_fundef(CAR(def->getRhs_c()))) {
	    // def is of the form _ <- function(...)
	    FundefCallGraphNode * node = cg->make_fundef_node(CAR(def->getRhs_c()));
	    cg->add_edge(this, node);
	  } else {
	    // RHS of def is a non-fundef; we don't know the function value
	    cg->add_edge(this, cg->make_unknown_value_node());
	    // TODO: if it's a variable, then we might know how to handle
	  }
	}
      }  
    }
  }

  void CoordinateCallGraphNode::
  get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
		    CallGraphAnnotationMap::NodeSetT & visited,
		    CallGraphAnnotation * ann) const
  {
    CallGraphAnnotationMap * cg = CallGraphAnnotationMap::get_instance();
    // TODO: remove cast when maps are parametrized
    CallGraphInfo * info = cg->get_edges(this);
    for (CallGraphInfo::const_iterator edge = info->begin_calls_out();
	 edge != info->end_calls_out();
	 ++edge) {
      worklist.push_back((*edge)->get_sink());
    }
  }

  void CoordinateCallGraphNode::
  dump(std::ostream & os) const {
    dump_string(os);
  }

  void CoordinateCallGraphNode::
  dump_string(std::ostream & os) const {
    const std::string name = var_name(m_name);
    const std::string scope_name = get_scope()->get_name();
    beginObjDump(os, CoordinateCallGraphNode);
    dumpVar(os, get_id());
    dumpString(os, name);
    dumpString(os, scope_name);
    endObjDump(os, CoordinateCallGraphNode);
  }

} // end namespace RAnnot
