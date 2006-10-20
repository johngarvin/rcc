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

// File: CallGraphAnnotationMap.cc
//
// Represents the call graph of the program. A call graph node can be
// a function definition ("fundef"), a call site, a coordinate (a name
// in a scope), an R library function, or an unknown value.
//
// Author: John Garvin (garvin@cs.rice.edu)

#include <list>

#include <support/DumpMacros.h>
#include <support/RccError.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/AnnotationBase.h>
#include <analysis/CallGraphAnnotation.h>
#include <analysis/CallGraphEdge.h>
#include <analysis/CallGraphNode.h>
#include <analysis/CallGraphInfo.h>
#include <analysis/FundefCallGraphNode.h>
#include <analysis/LibraryCallGraphNode.h>
#include <analysis/CoordinateCallGraphNode.h>
#include <analysis/CallSiteCallGraphNode.h>
#include <analysis/UnknownValueCallGraphNode.h>
#include <analysis/HandleInterface.h>
#include <analysis/SymbolTable.h>
#include <analysis/VarBinding.h>
#include <analysis/VarInfo.h>

#include "CallGraphAnnotationMap.h"

using namespace HandleInterface;

namespace RAnnot {

// ----- type definitions for readability -----
  
typedef CallGraphAnnotationMap::MyKeyT MyKeyT;
typedef CallGraphAnnotationMap::MyMappedT MyMappedT;
typedef CallGraphAnnotationMap::iterator iterator;
typedef CallGraphAnnotationMap::const_iterator const_iterator;

typedef CallGraphAnnotationMap::NodeListT NodeListT;
typedef CallGraphAnnotationMap::NodeSetT NodeSetT;
typedef CallGraphAnnotationMap::NodeMapT NodeMapT;
typedef CallGraphAnnotationMap::EdgeSetT EdgeSetT;

// ----- constructor, destructor -----

CallGraphAnnotationMap::CallGraphAnnotationMap()
  : m_computed(false), m_node_map(), m_edge_set(), m_traversed_map(),
    m_fundef_map(), m_library_map(), m_coord_map(), m_call_site_map()
{}
  
CallGraphAnnotationMap::~CallGraphAnnotationMap() {
  NodeMapT::const_iterator i;
  
  // delete CallGraphInfo objects
  for(i = m_node_map.begin(); i != m_node_map.end(); ++i) {
    delete(i->second);
  }
  
  // delete edges
  EdgeSetT::const_iterator j;
  for(j = m_edge_set.begin(); j != m_edge_set.end(); ++j) {
    delete(*j);
  }
}  
  
// ----- singleton pattern -----

CallGraphAnnotationMap * CallGraphAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}
  
PropertyHndlT CallGraphAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}
  
void CallGraphAnnotationMap::create() {
  m_instance = new CallGraphAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}
  
CallGraphAnnotationMap * CallGraphAnnotationMap::m_instance = 0;
PropertyHndlT CallGraphAnnotationMap::m_handle = "CallGraph";
  
//  ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & CallGraphAnnotationMap::operator[](const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  return m_traversed_map[k];
}

// Perform the computation if necessary and returns the requested
// data.
MyMappedT CallGraphAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  std::map<MyKeyT, MyMappedT>::const_iterator it = m_traversed_map.find(k);
  if (it != m_traversed_map.end()) {  // already computed and stored in map
    return it->second;
  } else {
    return get_call_bindings(k);
  }
}

CallGraphInfo* CallGraphAnnotationMap::get_edges(const CallGraphNode* node) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  return m_node_map[node];
}
  
bool CallGraphAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator CallGraphAnnotationMap::begin() { return m_traversed_map.begin(); }
iterator CallGraphAnnotationMap::end() { return m_traversed_map.end(); }
const_iterator CallGraphAnnotationMap::begin() const { return m_traversed_map.begin(); }
const_iterator CallGraphAnnotationMap::end() const { return m_traversed_map.end(); }

// ----- creation -----

FundefCallGraphNode * CallGraphAnnotationMap::make_fundef_node(SEXP e) {
  assert(is_fundef(e));
  FundefCallGraphNode * node;
  std::map<SEXP, FundefCallGraphNode *>::const_iterator it = m_fundef_map.find(e);
  if (it == m_fundef_map.end()) {  // not yet in map
    node = new FundefCallGraphNode(e);
    m_node_map[node] = new CallGraphInfo();
    m_fundef_map[e] = node;
  } else {
    node = it->second;
  }
  return node;
}

LibraryCallGraphNode * CallGraphAnnotationMap::make_library_node(SEXP name, SEXP value) {
  assert(is_var(name));
  assert(TYPEOF(value) == CLOSXP ||
	 TYPEOF(value) == BUILTINSXP ||
	 TYPEOF(value) == SPECIALSXP);
  LibraryCallGraphNode * node;
  std::map<SEXP, LibraryCallGraphNode *>::const_iterator it = m_library_map.find(name);
  if (it == m_library_map.end()) {  // not yet in map
    node = new LibraryCallGraphNode(name, value);
    m_node_map[node] = new CallGraphInfo();
    m_library_map[name] = node;
  } else {
    node = it->second;
  }
  return node;
}

CoordinateCallGraphNode * CallGraphAnnotationMap::make_coordinate_node(SEXP name, LexicalScope * scope) {
  assert(is_var(name));
  CoordinateCallGraphNode * node;
  std::map<std::pair<SEXP, LexicalScope *>, CoordinateCallGraphNode *>::const_iterator it;
  it = m_coord_map.find(std::make_pair(name, scope));
  if (it == m_coord_map.end()) {
    node = new CoordinateCallGraphNode(name, scope);
    m_node_map[node] = new CallGraphInfo();
    m_coord_map[std::make_pair(name, scope)] = node;
  } else {
    node = it->second;
  }
  return node;
}

CallSiteCallGraphNode * CallGraphAnnotationMap::make_call_site_node(SEXP e) {
  assert(is_call(e));
  CallSiteCallGraphNode * node;
  std::map<SEXP, CallSiteCallGraphNode *>::const_iterator it = m_call_site_map.find(e);
  if (it == m_call_site_map.end()) {  // not yet in map
    node = new CallSiteCallGraphNode(e);
    m_node_map[node] = new CallGraphInfo();
    m_call_site_map[e] = node;
  } else {
    node = it->second;
  }
  return node; 
}

UnknownValueCallGraphNode * CallGraphAnnotationMap::make_unknown_value_node() {
  UnknownValueCallGraphNode * node = UnknownValueCallGraphNode::get_instance();
  if (m_node_map.find(node) == m_node_map.end()) {
    m_node_map[node] = new CallGraphInfo();
  }
  return node;  
}

void CallGraphAnnotationMap::add_edge(const CallGraphNode * const source, const CallGraphNode * const sink) {
  CallGraphEdge * edge = new CallGraphEdge(source, sink);
  m_edge_set.insert(edge);
  CallGraphInfo * source_info = m_node_map[source];
  CallGraphInfo * sink_info = m_node_map[sink];
  assert(source_info != 0);
  assert(sink_info != 0);
  source_info->insert_call_out(edge);
  sink_info->insert_call_in(edge);
}

// ----- computation -----  

MyMappedT CallGraphAnnotationMap::get_call_bindings(MyKeyT cs) {
  const CallSiteCallGraphNode * const cs_node = make_call_site_node(make_sexp(cs));
  
  // search graph, accumulate fundefs/library functions
  NodeListT worklist;
  NodeSetT visited;
  
  // new annotation
  CallGraphAnnotation * ann = new CallGraphAnnotation();
  
  // start with worklist containing the given call site
  worklist.push_back(cs_node);
  
  while(!worklist.empty()) {
    const CallGraphNode * c = worklist.front();
    worklist.pop_front();
    if (visited.find(c) == visited.end()) {
      visited.insert(c);
      c->get_call_bindings(worklist, visited, ann);
    }
  }
  m_traversed_map[cs] = ann;
  return ann;
}

void CallGraphAnnotationMap::compute() {
  NodeListT worklist;
  NodeSetT visited;

  // add all fundefs to the worklist
  SEXP program = CAR(assign_rhs_c(R_Analyst::get_instance()->get_program()));
  FuncInfoIterator fii(getProperty(FuncInfo, program));
  for(FuncInfo *fi; fi = fii.Current(); ++fii) {
    worklist.push_back(make_fundef_node(fi->get_defn()));
  }

  while(!worklist.empty()) {
    const CallGraphNode * c = worklist.front();
    worklist.pop_front();
    if (visited.find(c) == visited.end()) {
      visited.insert(c);
      c->compute(worklist, visited);
    }
  }

  //  for each procedure:
  //    add to worklist as new call graph node
  //  for each node in worklist:
  //    mark node visited
  //    if it's a Fundef:
  //      for each call site:
  //        add edge (fundef, call site)
  //        if call site node hasn't been visited, add to worklist
  //    else if worklist item is a CallSite:
  //      if LHS is a simple name:
  //        for each scope in VarBinding:
  //          add name, scope as new coordinate node if not already present
  //          add edge (call site, coordinate)
  //          if coordinate node hasn't been visited, add to worklist
  //      else [LHS is not a simple name]:
  //        add edge (call site, unknown)
  //    else if worklist item is a Coordinate:
  //      look up name, scope in SymbolTable, get list of defs
  //      for each def:
  //        if RHS is a fundef:
  //          add edge (Coordinate, fundef)
  //        else:
  //          add edge (Coordinate, unknown) NOTE: look up DSystem's name
}

void CallGraphAnnotationMap::dump(std::ostream & os) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  beginObjDump(os, CallGraph);

  NodeMapT::const_iterator node_it;
  CallGraphInfo::const_iterator edge_it;
  const CallGraphEdge * edge;
  for(node_it = m_node_map.begin(); node_it != m_node_map.end(); ++node_it) {
    const CallGraphNode * node = node_it->first;
    assert(node != 0);
    dumpObj(os, node);
    CallGraphInfo * const info = node_it->second;
    assert(info != 0);
    info->dump(os);
    os << std::endl;
  }

  endObjDump(os, CallGraph);
}

void CallGraphAnnotationMap::dumpdot(std::ostream & os) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }

  os << "digraph CallGraph" << " {" << std::endl;

  // dump nodes
  NodeMapT::const_iterator node_it;
  const CallGraphNode * node;
  unsigned int node_num;
  for(node_it = m_node_map.begin(); node_it != m_node_map.end(); ++node_it) {
    node = node_it->first;
    node_num = node->get_id();
    os << node_num << " [ label=\"" << node_num << "\\n";
    std::ostringstream ss;
    node->dump_string(ss);
    os << escape(ss.str());
    os << "\\n\" ];" << std::endl;
    os.flush();
  }

  // dump edges
  EdgeSetT::const_iterator edge_it;
  unsigned int source_num, sink_num;
  for(edge_it = m_edge_set.begin(); edge_it != m_edge_set.end(); ++edge_it) {
    source_num = (*edge_it)->get_source()->get_id();
    sink_num = (*edge_it)->get_sink()->get_id();
    os << source_num << " -> " << sink_num << ";" << std::endl;
    os.flush();
  }

  os << "}" << std::endl;
  os.flush();
}

} // end namespace RAnnot
