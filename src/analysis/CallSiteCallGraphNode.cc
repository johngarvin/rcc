#include "CallSiteCallGraphNode.h"

#include <support/DumpMacros.h>

#include <analysis/HandleInterface.h>

namespace RAnnot {

  CallSiteCallGraphNode::CallSiteCallGraphNode(const SEXP cs)
    : m_cs(cs)
  {}

  CallSiteCallGraphNode::~CallSiteCallGraphNode()
  {}

  const OA::IRHandle CallSiteCallGraphNode::get_handle() const {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(m_cs));
  }
  const SEXP CallSiteCallGraphNode::get_sexp() const {
    return m_cs;
  }

  void CallSiteCallGraphNode::dump(std::ostream & os) const {
    beginObjDump(os, CallSiteCallGraphNode);
    dumpPtr(os, this);
    dumpSEXP(os, m_cs);
    endObjDump(os, CallSiteCallGraphNode);
  }
} // end namespace RAnnot
