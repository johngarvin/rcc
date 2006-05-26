// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_ANNOTATION_MAP_H
#define CALL_GRAPH_ANNOTATION_MAP_H

// Represents the call graph of the program. A call graph node can be
// a function definition ("fundef"), a call site, a coordinate (a name
// in a scope), or an R library function.

#include <map>
#include <list>
#include <set>
#include <ostream>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/AnnotationMap.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

class CallGraphEdge;
class CallGraphInfo;

class CallGraphAnnotationMap : public AnnotationMap
{
public:
  // ----- inner classes -----
  class CallGraphNode;
  class FundefCallGraphNode;
  class CallSiteCallGraphNode;
  class CoordinateCallGraphNode;
  class LibraryCallGraphNode;

  // ----- types -----
  typedef std::list<const CallGraphNode *>                 NodeListT;
  typedef std::set<const CallGraphNode *>                  NodeSetT;
  typedef std::map<const CallGraphNode *, CallGraphInfo *> NodeMapT;
  typedef std::set<const CallGraphEdge *>                  EdgeSetT;

  // ----- destructor -----
  virtual ~CallGraphAnnotationMap();

  // ----- demand-driven analysis -----

  MyMappedT & operator[](const MyKeyT & k); // FIXME: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // ----- implement singleton pattern -----

  static CallGraphAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // ----- iterators -----

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  // ----- debugging -----

  /// dump debugging information about the call graph
  void dump(std::ostream & os);

  /// dump the call graph in dot form
  void dumpdot(std::ostream & os);

private:
  // ----- implement singleton pattern -----

  // private constructor for singleton pattern
  CallGraphAnnotationMap();

  static CallGraphAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();

  // ----- creation methods -----
  //
  // return different derived classes of CallGraphNode. Create new if
  // needed; otherwise return existing node in a map.
  //
  FundefCallGraphNode * make_fundef_node(SEXP e);
  LibraryCallGraphNode * make_library_node(SEXP name, SEXP value);
  CoordinateCallGraphNode * make_coordinate_node(SEXP name, SEXP scope);
  CallSiteCallGraphNode * make_call_site_node(SEXP e);

  void add_edge(const CallGraphNode * const source, const CallGraphNode * const sink);
  
  // ----- compute call graph -----

  void compute();

  // ----- traverse call graph, get annotation -----

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
  std::map<std::pair<SEXP,SEXP>, CoordinateCallGraphNode *> m_coord_map;
  std::map<SEXP, CallSiteCallGraphNode *> m_call_site_map;
};

}  // end namespace RAnnot

#endif
