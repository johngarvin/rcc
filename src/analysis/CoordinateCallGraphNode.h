// -*- Mode: C++ -*-x

#ifndef COORDINATE_CALL_GRAPH_NODE_H
#define COORDINATE_CALL_GRAPH_NODE_H

#include <include/R/R_RInternals.h>

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class CoordinateCallGraphNode : public CallGraphNode {
public:
  explicit CoordinateCallGraphNode(const SEXP name, const SEXP scope);
  virtual ~CoordinateCallGraphNode();

  const OA::IRHandle get_handle() const;

  const SEXP get_name() const;
  const SEXP get_scope() const;
private:
  const SEXP m_name;
  const SEXP m_scope;
};

} // end namespace RAnnot

#endif
