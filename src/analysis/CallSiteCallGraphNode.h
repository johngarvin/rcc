// -*- Mode: C++ -*-

#ifndef CALL_SITE_CALL_GRAPH_NODE_H
#define CALL_SITE_CALL_GRAPH_NODE_H

#include <ostream>

#include <include/R/R_RInternals.h>

#include <analysis/CallGraphNode.h>

namespace RAnnot {

class CallSiteCallGraphNode : public CallGraphNode {
public:
  explicit CallSiteCallGraphNode(const SEXP def);
  virtual ~CallSiteCallGraphNode();

  const OA::IRHandle get_handle() const;
  const SEXP get_sexp() const;

  void dump(std::ostream & os) const;
private:
  const SEXP m_cs;
};

} // end namespace RAnnot

#endif
