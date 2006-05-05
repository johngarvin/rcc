#include "CoordinateCallGraphNode.h"

namespace RAnnot {

  CoordinateCallGraphNode::CoordinateCallGraphNode(SEXP name, SEXP scope)
    : m_name(name), m_scope(scope)
  {}

  CoordinateCallGraphNode::~CoordinateCallGraphNode()
  {}

} // end namespace RAnnot
