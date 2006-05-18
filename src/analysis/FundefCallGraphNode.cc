#include "FundefCallGraphNode.h"

#include <support/DumpMacros.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Annotation.h>
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

  void FundefCallGraphNode::dump(std::ostream & os) const {
    FuncInfo * finfo = getProperty(FuncInfo, m_fundef);
    SEXP first_name = finfo->getFirstName();

    beginObjDump(os, FundefCallGraphNode);
    dumpPtr(os, this);
    dumpSEXP(os, first_name);
    endObjDump(os, FundefCallGraphNode);
  }

} // end namespace RAnnot
