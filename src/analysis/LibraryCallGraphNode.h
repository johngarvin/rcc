// -*- Mode: C++ -*-

#ifndef LIBRARY_CALL_GRAPH_NODE_H
#define LIBRARY_CALL_GRAPH_NODE_H

#include <include/R/R_RInternals.h>

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class LibraryCallGraphNode : public CallGraphNode {
public:
  explicit LibraryCallGraphNode(const SEXP def);
  virtual ~LibraryCallGraphNode();

  const OA::IRHandle get_handle() const;
  const SEXP get_sexp() const;
private:
  const SEXP m_def;
};

} // end namespace RAnnot

#endif
