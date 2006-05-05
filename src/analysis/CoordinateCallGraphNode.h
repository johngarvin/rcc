// -*- Mode: C++ -*-x

#ifndef COORDINATE_CALL_GRAPH_NODE_H
#define COORDINATE_CALL_GRAPH_NODE_H

#include <include/R/R_RInternals.h>

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class CoordinateCallGraphNode : public CallGraphNode {
public:
  explicit CoordinateCallGraphNode(SEXP name, SEXP scope);
  virtual ~CoordinateCallGraphNode();
private:
  SEXP m_name;
  SEXP m_scope;
};

} // end namespace RAnnot

#endif
