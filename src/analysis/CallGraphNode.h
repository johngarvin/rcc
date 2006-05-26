// -*-Mode: C++;-*-

#ifndef CALL_GRAPH_NODE_H
#define CALL_GRAPH_NODE_H

#include <ostream>

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <analysis/CallGraphAnnotationMap.h>

namespace RAnnot {

class CallGraphAnnotation;

/// Abstract class representing a node of a call graph
class CallGraphAnnotationMap::CallGraphNode {
public:
  explicit CallGraphNode();
  virtual ~CallGraphNode();

  virtual const OA::IRHandle get_handle() const = 0;

  virtual void dump(std::ostream & os) const = 0;
  virtual void dump_string(std::ostream & os) const = 0;

  /// compute the outgoing edges and add them to the graph
  virtual void compute(CallGraphAnnotationMap::NodeListT & worklist,
		       CallGraphAnnotationMap::NodeSetT & visited) const = 0;

  /// perform this node's part in traversing the graph to build bindings of a call site
  virtual void get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
				 CallGraphAnnotationMap::NodeSetT & visited,
				 CallGraphAnnotation * ann) const = 0;

  /// get the unique node ID
  int get_id() const;
private:
  // prevent copying
  CallGraphNode & operator=(const CallGraphNode &);
  CallGraphNode(const CallGraphNode &);

  int m_id;
  static int m_max_id;
};

} // end namespace RAnnot

#endif
