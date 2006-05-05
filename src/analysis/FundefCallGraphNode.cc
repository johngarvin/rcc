#include "FundefCallGraphNode.h"

namespace RAnnot {

  FundefCallGraphNode::FundefCallGraphNode(SEXP fundef)
    : m_fundef(fundef)
  {}

  FundefCallGraphNode::~FundefCallGraphNode()
  {}

} // end namespace RAnnot
