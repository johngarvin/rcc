// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_EDGE_H
#define CALL_GRAPH_EDGE_H

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class CallGraphEdge {
public:
  explicit CallGraphEdge(const CallGraphNode * const source, const CallGraphNode * const sink);
  virtual ~CallGraphEdge();
private:
  const CallGraphNode * const m_source;
  const CallGraphNode * const m_sink;
};

} // end namespace RAnnot

#endif
