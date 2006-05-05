// -*-Mode: C++;-*-

#ifndef CALL_GRAPH_NODE_H
#define CALL_GRAPH_NODE_H

/// Abstract class representing a node of a call graph

namespace RAnnot {

class CallGraphNode {
public:
  explicit CallGraphNode();
  virtual ~CallGraphNode();
private:
  // prevent copying
  CallGraphNode & operator=(const CallGraphNode &);
  CallGraphNode(const CallGraphNode &);
};

} // end namespace RAnnot

#endif
