#include "CallSiteCallGraphNode.h"

#include <support/DumpMacros.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/CallGraphInfo.h>
#include <analysis/CoordinateCallGraphNode.h>
#include <analysis/UnknownValueCallGraphNode.h>
#include <analysis/HandleInterface.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

namespace RAnnot {

  CallGraphAnnotationMap::CallSiteCallGraphNode::CallSiteCallGraphNode(const SEXP cs)
    : m_cs(cs)
  {}

  CallGraphAnnotationMap::CallSiteCallGraphNode::~CallSiteCallGraphNode()
  {}

  const OA::IRHandle CallGraphAnnotationMap::CallSiteCallGraphNode::get_handle() const {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(m_cs));
  }
  const SEXP CallGraphAnnotationMap::CallSiteCallGraphNode::get_sexp() const {
    return m_cs;
  }

  void CallGraphAnnotationMap::CallSiteCallGraphNode::
  compute(CallGraphAnnotationMap::NodeListT & worklist,
	  CallGraphAnnotationMap::NodeSetT & visited) const
  {
    CallGraphAnnotationMap * cg = CallGraphAnnotationMap::get_instance();
    if (is_var(call_lhs(m_cs))) {
      // if left side of call is a symbol
      VarBinding * binding = getProperty(VarBinding, m_cs);  // cell containing symbol
      VarBinding::const_iterator si;
      for(si = binding->begin(); si != binding->end(); ++si) {
	CoordinateCallGraphNode * node;
	node = cg->make_coordinate_node(call_lhs(m_cs), (*si)->get_defn());
	cg->add_edge(this, node);
	if (visited.find(node) == visited.end()) {
	  worklist.push_back(node);
	}
      }
    } else {
      // LHS of call is a non-symbol expression
      cg->add_edge(this, cg->make_unknown_value_node());
    }
  }

  void CallGraphAnnotationMap::CallSiteCallGraphNode::
  get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
		    CallGraphAnnotationMap::NodeSetT & visited,
		    CallGraphAnnotation * ann) const
  {
    CallGraphAnnotationMap * cg = CallGraphAnnotationMap::get_instance();
    CallGraphInfo * info = cg->get_edges(this);
    // TODO: remove cast when maps are parametrized
    for (CallGraphInfo::const_iterator edge = info->begin_calls_out();
	 edge != info->end_calls_out();
	 ++edge) {
      assert(dynamic_cast<const CoordinateCallGraphNode*>((*edge)->get_sink()) != 0);
      worklist.push_back((*edge)->get_sink());
    }
  }

  void CallGraphAnnotationMap::CallSiteCallGraphNode::dump(std::ostream & os) const {
    beginObjDump(os, CallSiteCallGraphNode);
    dumpPtr(os, this);
    dumpSEXP(os, m_cs);
    endObjDump(os, CallSiteCallGraphNode);
  }

  // TODO: find a way to print the actual call site without the
  // limitations of R printing
  void CallGraphAnnotationMap::CallSiteCallGraphNode::dump_string(std::ostream & os) const {
    beginObjDump(os, CallSiteCallGraphNode);
    dumpVar(os, get_id());
    endObjDump(os, CallSiteCallGraphNode);
  }

} // end namespace RAnnot
