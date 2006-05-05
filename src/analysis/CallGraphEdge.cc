#include "CallGraphEdge.h"

namespace RAnnot {

  CallGraphEdge::CallGraphEdge(const CallGraphNode * const source, const CallGraphNode * const sink)
    : m_source(source), m_sink(sink)
  {}

  CallGraphEdge::~CallGraphEdge()
  {}

} // end namespace RAnnot
