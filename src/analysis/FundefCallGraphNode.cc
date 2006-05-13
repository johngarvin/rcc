#include "FundefCallGraphNode.h"

#include <analysis/HandleInterface.h>

namespace RAnnot {

  FundefCallGraphNode::FundefCallGraphNode(const SEXP fundef)
    : m_fundef(fundef)
  {}

  FundefCallGraphNode::~FundefCallGraphNode()
  {}

  const OA::IRHandle FundefCallGraphNode::get_handle() const {
    return HandleInterface::make_proc_h(m_fundef);
  }

  const SEXP FundefCallGraphNode::get_sexp() const {
    return m_fundef;
  }

} // end namespace RAnnot
