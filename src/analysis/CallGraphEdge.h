// -*- Mode: C++ -*-

#ifndef CALL_GRAPH_EDGE_H
#define CALL_GRAPH_EDGE_H

namespace RAnnot {

class CallGraphNode;

class CallGraphEdge {
public:
  explicit CallGraphEdge(const CallGraphNode * const source,
			 const CallGraphNode * const sink);
  virtual ~CallGraphEdge();

  const CallGraphNode * const get_source() const;
  const CallGraphNode * const get_sink() const;

private:
  const CallGraphNode * const m_source;
  const CallGraphNode * const m_sink;
};

} // end namespace RAnnot

#endif
