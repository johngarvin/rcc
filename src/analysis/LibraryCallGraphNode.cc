#include "LibraryCallGraphNode.h"

#include <support/DumpMacros.h>

#include <analysis/HandleInterface.h>

namespace RAnnot {

  LibraryCallGraphNode::LibraryCallGraphNode(const SEXP name, const SEXP value)
    : m_name(name), m_value(value)
  {}

  LibraryCallGraphNode::~LibraryCallGraphNode()
  {}

  const OA::IRHandle LibraryCallGraphNode::get_handle() const {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(m_name));
  }
  const SEXP LibraryCallGraphNode::get_name() const {
    return m_name;
  }
  const SEXP LibraryCallGraphNode::get_value() const {
    return m_value;
  }

  void LibraryCallGraphNode::dump(std::ostream & os) const {
    beginObjDump(os, LibraryCallGraphNode);
    dumpPtr(os, this);
    dumpSEXP(os, m_name);
    endObjDump(os, LibraryCallGraphNode);
  }
} // end namespace RAnnot
