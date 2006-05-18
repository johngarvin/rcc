#include <support/DumpMacros.h>

#include <analysis/AnalysisResults.h>

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

  void CoordinateCallGraphNode::dump(std::ostream & os) const {
    FuncInfo * finfo = getProperty(FuncInfo, m_scope);
    SEXP scope_first_name = finfo->getFirstName();

    beginObjDump(os, CoordinateCallGraphNode);
    dumpPtr(os, this);
    dumpSEXP(os, m_name);
    dumpSEXP(os, scope_first_name);
    endObjDump(os, CoordinateCallGraphNode);
  }

} // end namespace RAnnot
