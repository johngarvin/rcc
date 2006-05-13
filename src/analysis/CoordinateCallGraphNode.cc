#include "CoordinateCallGraphNode.h"

namespace RAnnot {

  CoordinateCallGraphNode::CoordinateCallGraphNode(SEXP name, SEXP scope)
    : m_name(name), m_scope(scope)
  {}

  CoordinateCallGraphNode::~CoordinateCallGraphNode()
  {}

  const SEXP CoordinateCallGraphNode::get_name() const {
    return m_name;
  }

  const SEXP CoordinateCallGraphNode::get_scope() const {
    return m_scope;
  }

  const OA::IRHandle CoordinateCallGraphNode::get_handle() const {
    return reinterpret_cast<OA::irhandle_t>(this);
  }

} // end namespace RAnnot
