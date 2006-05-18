#include "CallGraphEdge.h"

namespace RAnnot {

  CallGraphEdge::CallGraphEdge(const CallGraphNode * const source,
			       const CallGraphNode * const sink)
    : m_source(source), m_sink(sink)
  {}

  CallGraphEdge::~CallGraphEdge()
  {}

  const CallGraphNode * const CallGraphEdge::get_source() const {
    return m_source;
  }

  const CallGraphNode * const CallGraphEdge::get_sink() const {
    return m_sink;
  }


} // end namespace RAnnot
