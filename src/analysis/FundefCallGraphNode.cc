#include <support/DumpMacros.h>

#include <analysis/AnalysisResults.h>
#include <analysis/CallGraphAnnotation.h>
#include <analysis/CallSiteCallGraphNode.h>
#include <analysis/FuncInfo.h>
#include <analysis/HandleInterface.h>

#include "FundefCallGraphNode.h"

namespace RAnnot {

  CallGraphAnnotationMap::FundefCallGraphNode::FundefCallGraphNode(const SEXP fundef)
    : m_fundef(fundef)
  {}

  CallGraphAnnotationMap::FundefCallGraphNode::~FundefCallGraphNode()
  {}

  const OA::IRHandle CallGraphAnnotationMap::FundefCallGraphNode::get_handle() const {
    return HandleInterface::make_proc_h(m_fundef);
  }

  const SEXP CallGraphAnnotationMap::FundefCallGraphNode::get_sexp() const {
    return m_fundef;
  }

  void CallGraphAnnotationMap::FundefCallGraphNode::
  compute(CallGraphAnnotationMap::NodeListT & worklist,
	  CallGraphAnnotationMap::NodeSetT & visited) const
  {
    CallGraphAnnotationMap * const cg = CallGraphAnnotationMap::get_instance();
    FuncInfo * fi = getProperty(FuncInfo, m_fundef);

    // Keep track of call sites attached to this node to avoid
    // redundancy. This is a temporary workaround for a bug in which
    // the CFG has duplicate nodes.
    NodeSetT visited_cs;

    const CallSiteCallGraphNode * csnode;
    FuncInfo::const_call_site_iterator csi;
    for(csi = fi->begin_call_sites(); csi != fi->end_call_sites(); ++csi) {
      SEXP cs = *csi;
      csnode = cg->make_call_site_node(cs);
      if (visited_cs.find(csnode) == visited_cs.end()) {
	visited_cs.insert(csnode);
	cg->add_edge(this, csnode);
	if (visited.find(csnode) == visited.end()) {
	  worklist.push_back(csnode);
	}
      }
    }
  }

  void CallGraphAnnotationMap::FundefCallGraphNode::
  get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
		    CallGraphAnnotationMap::NodeSetT & visited,
		    CallGraphAnnotation * ann) const
  {
    ann->insert_node(this);
  }

  void CallGraphAnnotationMap::FundefCallGraphNode::dump(std::ostream & os) const {
    FuncInfo * finfo = getProperty(FuncInfo, m_fundef);
    SEXP first_name = finfo->get_first_name();

    beginObjDump(os, FundefCallGraphNode);
    dumpPtr(os, this);
    dumpSEXP(os, first_name);
    endObjDump(os, FundefCallGraphNode);
  }

  void CallGraphAnnotationMap::FundefCallGraphNode::dump_string(std::ostream & os) const {
    FuncInfo * finfo = getProperty(FuncInfo, m_fundef);
    std::string first_name = CHAR(PRINTNAME(finfo->get_first_name()));
    beginObjDump(os, FundefCallGraphNode);
    dumpVar(os, get_id());
    dumpString(os, first_name);
    endObjDump(os, FundefCallGraphNode);
  }

} // end namespace RAnnot
