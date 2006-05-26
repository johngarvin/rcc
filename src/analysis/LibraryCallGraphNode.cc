#include "LibraryCallGraphNode.h"

#include <support/DumpMacros.h>
#include <support/RccError.h>

#include <analysis/CallGraphAnnotation.h>
#include <analysis/HandleInterface.h>

namespace RAnnot {

  CallGraphAnnotationMap::LibraryCallGraphNode::LibraryCallGraphNode(const SEXP name, const SEXP value)
    : m_name(name), m_value(value)
  {}

  CallGraphAnnotationMap::LibraryCallGraphNode::~LibraryCallGraphNode()
  {}

  const OA::IRHandle CallGraphAnnotationMap::LibraryCallGraphNode::get_handle() const {
    return OA::IRHandle(reinterpret_cast<OA::irhandle_t>(m_name));
  }
  const SEXP CallGraphAnnotationMap::LibraryCallGraphNode::get_name() const {
    return m_name;
  }
  const SEXP CallGraphAnnotationMap::LibraryCallGraphNode::get_value() const {
    return m_value;
  }

  void CallGraphAnnotationMap::LibraryCallGraphNode::
  compute(CallGraphAnnotationMap::NodeListT & worklist,
	  CallGraphAnnotationMap::NodeSetT & visited) const
  {
    rcc_error("compute: Library call graph node incorrectly appeared in worklist");
  }

  void CallGraphAnnotationMap::LibraryCallGraphNode::
  get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
		    CallGraphAnnotationMap::NodeSetT & visited,
		    CallGraphAnnotation * ann) const
  {
    ann->insert_node(this);
  }

  void CallGraphAnnotationMap::LibraryCallGraphNode::dump(std::ostream & os) const {
    beginObjDump(os, LibraryCallGraphNode);
    dumpPtr(os, this);
    dumpSEXP(os, m_name);
    endObjDump(os, LibraryCallGraphNode);
  }

  void CallGraphAnnotationMap::LibraryCallGraphNode::dump_string(std::ostream & os) const {
    std::string first_name = CHAR(PRINTNAME(m_name));
    beginObjDump(os, LibraryCallGraphNode);
    dumpVar(os, get_id());
    dumpString(os, first_name);
    endObjDump(os, LibraryCallGraphNode);
  }

} // end namespace RAnnot
