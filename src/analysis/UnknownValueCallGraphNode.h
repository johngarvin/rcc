// -*- Mode: C++ -*-

#ifndef UNKNOWN_VALUE_CALL_GRAPH_NODE_H
#define UNKNOWN_VALUE_CALL_GRAPH_NODE_H

/// UnknownValueCallGraphNode represents an unknown function value in
/// the call graph. Incoming edges come from Coordinate nodes; no
/// edges go out. This class is a singleton because there's no reason
/// to distinguish different instances.

#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class CallGraphAnnotationMap::UnknownValueCallGraphNode : public CallGraphAnnotationMap::CallGraphNode {
public:
  virtual ~UnknownValueCallGraphNode();

  const OA::IRHandle get_handle() const;

  void compute(CallGraphAnnotationMap::NodeListT & worklist,
	       CallGraphAnnotationMap::NodeSetT & visited) const;

  void get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
			 CallGraphAnnotationMap::NodeSetT & visited,
			 CallGraphAnnotation * ann) const;

  void dump(std::ostream & os) const;
  void dump_string(std::ostream & os) const;

  static UnknownValueCallGraphNode * get_instance();

private:
  explicit UnknownValueCallGraphNode();
  static UnknownValueCallGraphNode * m_instance;
};

} // end namespace RAnnot

#endif
