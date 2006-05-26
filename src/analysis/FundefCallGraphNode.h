// -*- Mode: C++ -*-

#ifndef FUNDEF_CALL_GRAPH_NODE_H
#define FUNDEF_CALL_GRAPH_NODE_H

#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class CallGraphAnnotationMap::FundefCallGraphNode : public CallGraphNode {
public:
  explicit FundefCallGraphNode(const SEXP fundef);
  virtual ~FundefCallGraphNode();

  const OA::IRHandle get_handle() const;
  const SEXP get_sexp() const;

  void compute(CallGraphAnnotationMap::NodeListT & worklist,
	       CallGraphAnnotationMap::NodeSetT & visited) const;

  void get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
			 CallGraphAnnotationMap::NodeSetT & visited,
			 CallGraphAnnotation * ann) const;

  /// dump the node contents to os using R printing
  void dump(std::ostream & os) const;

  /// dump the node contents to os as a pure string
  void dump_string(std::ostream & os) const;
private:
  const SEXP m_fundef;
};

} // end namespace RAnnot

#endif
