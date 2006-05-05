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

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/AnnotationMap.h>
#include <analysis/CallGraphEdge.h>
#include <analysis/CallGraphInfo.h>
#include <analysis/CallGraphNode.h>

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

private:
  // private constructor for singleton pattern
  CallGraphAnnotationMap();

  void compute();

  bool m_computed; // has our information been computed yet?

  static CallGraphAnnotationMap * m_instance;
  static PropertyHndlT m_handle;
  static void create();
  
private:
  std::map<MyKeyT, MyMappedT> m_node_map;
  std::set<const CallGraphEdge *> m_edge_set;
};

}  // end namespace RAnnot

#endif
