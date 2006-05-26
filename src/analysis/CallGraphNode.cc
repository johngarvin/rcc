#include "CallGraphNode.h"

namespace RAnnot {

  CallGraphAnnotationMap::CallGraphNode::CallGraphNode() : m_id(m_max_id++)
  {}

  CallGraphAnnotationMap::CallGraphNode::~CallGraphNode()
  {}

  int CallGraphAnnotationMap::CallGraphNode::get_id() const {
    return m_id;
  }

  int CallGraphAnnotationMap::CallGraphNode::m_max_id = 0;

} // end namespace RAnnot
