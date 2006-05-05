// -*- Mode: C++ -*-

#ifndef FUNDEF_CALL_GRAPH_NODE_H
#define FUNDEF_CALL_GRAPH_NODE_H

#include <include/R/R_RInternals.h>

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class FundefCallGraphNode : public CallGraphNode {
public:
  explicit FundefCallGraphNode(SEXP fundef);
  virtual ~FundefCallGraphNode();
private:
  SEXP m_fundef;
};

} // end namespace RAnnot

#endif
