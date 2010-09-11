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

// File: RccCallGraphAnnotationMap.h
//
// Represents the call graph of the program. A call graph node can be
// a function definition ("fundef"), a call site, a coordinate (a name
// in a scope), an R library function, or an unknown value.
//
// Author: John Garvin (garvin@cs.rice.edu)

#ifndef RCC_CALL_GRAPH_ANNOTATION_MAP_H
#define RCC_CALL_GRAPH_ANNOTATION_MAP_H

#include <map>
#include <list>
#include <set>
#include <ostream>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/DefaultAnnotationMap.h>
#include <analysis/LexicalScope.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class CallGraphEdge;
class CallGraphInfo;
class CallGraphNode;
class CallSiteCallGraphNode;
class CoordinateCallGraphNode;
class FundefCallGraphNode;
class LibraryCallGraphNode;
class UnknownValueCallGraphNode;

class RccCallGraphAnnotationMap : public DefaultAnnotationMap
{
public:
  // ----- types -----
  typedef std::list<const CallGraphNode *>                 NodeListT;
  typedef std::set<const CallGraphNode *>                  NodeSetT;
  typedef std::map<const CallGraphNode *, CallGraphInfo *> NodeMapT;
  typedef std::set<const CallGraphEdge *>                  EdgeSetT;

  // ----- destructor -----
  virtual ~RccCallGraphAnnotationMap();

  // ----- demand-driven analysis -----

  // overrides DefaultAnnotationMap
  MyMappedT & operator[](const MyKeyT & k); // TODO: remove this when refactoring is done

  // overrides DefaultAnnotationMap
  /// given a call site, return the list of fundef/library nodes reachable; compute if necessary
  MyMappedT get(const MyKeyT & k);

  /// given a node, return a CallGraphInfo containing the in and out edges
  CallGraphInfo * get_edges(const CallGraphNode * node);

  // ----- implement singleton pattern -----

  static RccCallGraphAnnotationMap * instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // ----- debugging -----

  /// dump debugging information about the call graph
  void dump(std::ostream & os);

  /// dump the call graph in dot form
  void dumpdot(std::ostream & os);

private:
  // ----- implement singleton pattern -----

  // private constructor for singleton pattern
  explicit RccCallGraphAnnotationMap();

  static RccCallGraphAnnotationMap * s_instance;
  static PropertyHndlT s_handle;
  static void create();

  // ----- creation methods -----
  //
  // return different derived classes of CallGraphNode. Create new if
  // needed; otherwise return existing node in a map.
  //
public:
  FundefCallGraphNode * make_fundef_node(SEXP e);
  LibraryCallGraphNode * make_library_node(SEXP name, SEXP value);
  CoordinateCallGraphNode * make_coordinate_node(SEXP name, LexicalScope * scope);
  CallSiteCallGraphNode * make_call_site_node(SEXP e);
  UnknownValueCallGraphNode * make_unknown_value_node();

  void add_edge(const CallGraphNode * const source, const CallGraphNode * const sink);
  
  // ----- compute call graph -----

private:
  void compute();

  // ----- traverse call graph, get annotation -----

private:
  /// given a call site, traverse call graph to find the functions to
  /// which the left side may be bound. Specifically: let a
  /// _definition_ node be a FundefCallGraphNode or
  /// LibraryCallGraphNode. get_call_bindings finds all definition
  /// nodes that can be reached from the call site by a path that
  /// includes exactly one definition node.
  MyMappedT get_call_bindings(MyKeyT cs);
  

private:
  bool m_computed; // has our information been computed yet?
  NodeMapT m_node_map; // owns nodes and node info
  EdgeSetT m_edge_set;              // owns edges for memory purposes

  std::map<MyKeyT, MyMappedT> m_traversed_map; // stores info on problems we've solved

  std::map<SEXP, FundefCallGraphNode *> m_fundef_map;
  std::map<SEXP, LibraryCallGraphNode *> m_library_map;
  std::map<std::pair<SEXP, LexicalScope * >, CoordinateCallGraphNode *> m_coord_map;
  std::map<SEXP, CallSiteCallGraphNode *> m_call_site_map;
};

}  // end namespace RAnnot

#endif
