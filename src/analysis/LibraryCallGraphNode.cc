#include "LibraryCallGraphNode.h"

#include <analysis/HandleInterface.h>

namespace RAnnot {

  LibraryCallGraphNode::LibraryCallGraphNode(const SEXP def)
    : m_def(def)
  {}

  LibraryCallGraphNode::~LibraryCallGraphNode()
  {}

  const OA::IRHandle LibraryCallGraphNode::get_handle() const {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(m_def));
  }
  const SEXP LibraryCallGraphNode::get_sexp() const {
    return m_def;
  }

} // end namespace RAnnot
