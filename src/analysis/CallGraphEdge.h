// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_EDGE_H
#define CALL_GRAPH_EDGE_H

#include <analysis/CallGraphAnnotationMap.h>

namespace RAnnot {

  // class CallGraphAnnotationMap::CallGraphNode;  // why doesn't this work?

class CallGraphEdge {
public:
  explicit CallGraphEdge(const CallGraphAnnotationMap::CallGraphNode * const source,
			 const CallGraphAnnotationMap::CallGraphNode * const sink);
  virtual ~CallGraphEdge();

  const CallGraphAnnotationMap::CallGraphNode * const get_source() const;
  const CallGraphAnnotationMap::CallGraphNode * const get_sink() const;

private:
  const CallGraphAnnotationMap::CallGraphNode * const m_source;
  const CallGraphAnnotationMap::CallGraphNode * const m_sink;
};

} // end namespace RAnnot

#endif
