#include <support/DumpMacros.h>
#include <support/RccError.h>

#include <analysis/AnalysisResults.h>
#include <analysis/CallGraphAnnotation.h>
#include <analysis/CallGraphInfo.h>
#include <analysis/Utils.h>

#include "UnknownValueCallGraphNode.h"

namespace RAnnot {

  CallGraphAnnotationMap::UnknownValueCallGraphNode::UnknownValueCallGraphNode()
  {}

  CallGraphAnnotationMap::UnknownValueCallGraphNode::~UnknownValueCallGraphNode()
  {}

  const OA::IRHandle CallGraphAnnotationMap::UnknownValueCallGraphNode::get_handle() const
  {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(get_instance()));
  }

  void CallGraphAnnotationMap::UnknownValueCallGraphNode::
  compute(CallGraphAnnotationMap::NodeListT & worklist,
	  CallGraphAnnotationMap::NodeSetT & visited) const
  {
    rcc_error("Internal error: compute: UnknownValueCallGraphNode found on worklist");
  }

  void CallGraphAnnotationMap::UnknownValueCallGraphNode::
  get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
		    CallGraphAnnotationMap::NodeSetT & visited,
		    CallGraphAnnotation * ann) const
  {
    ann->insert_node(this);
  }

  void CallGraphAnnotationMap::UnknownValueCallGraphNode::
  dump(std::ostream & os) const {
    beginObjDump(os, UnknownValueCallGraphNode);
    dumpVar(os, get_id());
    endObjDump(os, UnknownValueCallGraphNode);
  }

  void CallGraphAnnotationMap::UnknownValueCallGraphNode::
  dump_string(std::ostream & os) const {
    beginObjDump(os, UnknownValueCallGraphNode);
    dumpVar(os, get_id());
    endObjDump(os, UnknownValueCallGraphNode);
  }
  
  CallGraphAnnotationMap::UnknownValueCallGraphNode * CallGraphAnnotationMap::UnknownValueCallGraphNode::
  get_instance() {
    if (m_instance == 0) {
      m_instance = new UnknownValueCallGraphNode();
    }
    return m_instance;
  }
  
  CallGraphAnnotationMap::UnknownValueCallGraphNode * CallGraphAnnotationMap::UnknownValueCallGraphNode::
  m_instance = 0;

} // end RAnnot namespace
