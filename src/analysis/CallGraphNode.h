// -*-Mode: C++;-*-

#ifndef CALL_GRAPH_NODE_H
#define CALL_GRAPH_NODE_H

#include <OpenAnalysis/IRInterface/IRHandles.hpp>

/// Abstract class representing a node of a call graph

namespace RAnnot {

class CallGraphNode {
public:
  explicit CallGraphNode();
  virtual ~CallGraphNode();

  virtual const OA::IRHandle get_handle() const = 0;

private:
  // prevent copying
  CallGraphNode & operator=(const CallGraphNode &);
  CallGraphNode(const CallGraphNode &);
};

} // end namespace RAnnot

#endif
