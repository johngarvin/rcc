// -*- Mode: C++ -*-

#ifndef LIBRARY_CALL_GRAPH_NODE_H
#define LIBRARY_CALL_GRAPH_NODE_H

#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class CallGraphAnnotationMap::LibraryCallGraphNode : public CallGraphAnnotationMap::CallGraphNode {
public:
  explicit LibraryCallGraphNode(const SEXP name, const SEXP value);
  virtual ~LibraryCallGraphNode();

  const OA::IRHandle get_handle() const;
  const SEXP get_name() const;
  const SEXP get_value() const;

  void compute(CallGraphAnnotationMap::NodeListT & worklist,
	       CallGraphAnnotationMap::NodeSetT & visited) const;

  void get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
			 CallGraphAnnotationMap::NodeSetT & visited,
			 CallGraphAnnotation * ann) const;

  void dump(std::ostream & os) const;
  void dump_string(std::ostream & os) const;
private:
  const SEXP m_name;
  const SEXP m_value;
};

} // end namespace RAnnot

#endif
