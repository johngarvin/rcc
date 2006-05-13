// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_ANNOTATION_MAP_H
#define CALL_GRAPH_ANNOTATION_MAP_H

// Represents the call graph of the program. To represent R's binding
// semantics, a call graph node can be a function definition
// ("fundef") or a coordinate (a name in a scope). This system
// reflects the fact that a call site in R can refer to more than one
// coordinate, and a coordinate can refer to more than one fundef
// (i.e., a name in a scope can be bound to more than one function).

#include <map>
#include <set>
#include <ostream>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/AnnotationMap.h>
#include <analysis/CallGraphEdge.h>
#include <analysis/CallGraphInfo.h>
#include <analysis/CallGraphNode.h>
#include <analysis/CoordinateCallGraphNode.h>
#include <analysis/FundefCallGraphNode.h>
#include <analysis/LibraryCallGraphNode.h>

namespace RAnnot {

class CallGraphAnnotationMap : public AnnotationMap
{
public:
  virtual ~CallGraphAnnotationMap();

  // demand-driven analysis
  MyMappedT & operator[](const MyKeyT & k); // FIXME: remove this when refactoring is done
  MyMappedT get(const MyKeyT & k);
  bool is_computed();

  // singleton
  static CallGraphAnnotationMap * get_instance();

  // getting the name causes this map to be created and registered
  static PropertyHndlT handle();

  // iterators
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  void dump(std::ostream & stream);

private:
  // private constructor for singleton pattern
  CallGraphAnnotationMap();

  void compute();

  static CallGraphAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();

  FundefCallGraphNode * make_fundef_node(SEXP e);
  LibraryCallGraphNode * make_library_node(SEXP e);
  CoordinateCallGraphNode * make_coordinate_node(SEXP name, SEXP scope);
  void add_edge(CallGraphNode * source_node, SEXP source_call_site, CallGraphNode * sink);
  void add_edge(CoordinateCallGraphNode * source, CallGraphNode * sink);
  
private:
  bool m_computed; // has our information been computed yet?
  std::map<MyKeyT, MyMappedT> m_node_map;
  std::set<const CallGraphEdge *> m_edge_set;
  std::map<SEXP, FundefCallGraphNode *> m_fundef_map;
  std::map<SEXP, LibraryCallGraphNode *> m_library_map;
  std::map<std::pair<SEXP,SEXP>, CoordinateCallGraphNode *> m_coord_map;
};

}  // end namespace RAnnot

#endif
