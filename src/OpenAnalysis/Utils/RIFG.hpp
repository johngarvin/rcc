/*! \file
  
  \brief Representation Independent Flowgraph Interface.  This is a
         wrapper that allows access to an OA DGraph through dense id
         numbers ranging from 1 to n.  It is useful to interface with
         algorithms that worked by creating a work or result array of
         size n.  It might more appropriately be a mixin to an OA
         DGraph.
        
  \authors Original DSystem code by John Mellor-Crummey
           Port from 'Classic OpenAnalysis' to OpenAnalysis by Nathan
             Tallent (merged RIFG and OARIFG)
  \version $Id: RIFG.hpp,v 1.4 2005/06/10 02:32:05 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef RepresentationIndependentFlowGraph_H
#define RepresentationIndependentFlowGraph_H

#include <iostream>
#include <map>

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Utils/DGraph/DGraphInterface.hpp>

//***************************************************************************

namespace OA {
  
  class RIFGNodeIterator;
  class RIFGEdgeIterator;

//***************************************************************************
// Class: Representation Independent Flowgraph Interface
//
// Description:
//    A wrapper that allows access to an OA DGraph through dense id
//    numbers ranging from 1 to n.  It is useful to interface with
//    algorithms that worked by creating a work or result array of
//    size n.  It might more appropriately be a mixin to an OA DGraph.
//
//***************************************************************************

class RIFG {
public:

  typedef unsigned int NodeId; // use RIFG::NIL for null value
  typedef unsigned int EdgeId; // use RIFG::NIL for null value

  static unsigned int NIL;

  
  typedef std::map<NodeId, OA::OA_ptr<OA::DGraph::NodeInterface> > 
    IdToNodeMap_t;
  typedef std::map<OA::OA_ptr<OA::DGraph::NodeInterface>, NodeId>
    NodeToIdMap_t;
  
  typedef std::map<EdgeId, OA::OA_ptr<OA::DGraph::EdgeInterface> > 
    IdToEdgeMap_t;
  typedef std::map<OA::OA_ptr<OA::DGraph::EdgeInterface>, EdgeId>
    EdgeToIdMap_t;

public:

  class NodesIterator;
  class IncomingEdgesIterator;
  class OutgoingEdgesIterator;

  class NodesIterator {
  public:
    NodesIterator(const RIFG& rifg_)
      : rifg(rifg_), it(rifg.graph->getNodesIterator()) { }
    ~NodesIterator() { }
    
    RIFG::NodeId current() const {
      OA::OA_ptr<OA::DGraph::NodeInterface> node = it->current();
      return rifg.getNodeId(node);
    }
    void operator++() { ++(*it); }
    bool isValid() const { return it->isValid(); }
    void reset() { it->reset(); }
    
  private:
    const RIFG& rifg;
    OA_ptr<OA::DGraph::NodesIteratorInterface> it;
  };


  class IncomingEdgesIterator {
  public:
    IncomingEdgesIterator(const RIFG& rifg_, RIFG::NodeId nid)
      : rifg(rifg_) 
    {
      OA::OA_ptr<OA::DGraph::NodeInterface> n = rifg.getNode(nid);
      it = n->getIncomingEdgesIterator();
    }
    ~IncomingEdgesIterator() { }
    
    RIFG::EdgeId current() const {
      OA::OA_ptr<OA::DGraph::EdgeInterface> edge = it->current();
      return rifg.getEdgeId(edge);
    }
    void operator++() { ++(*it); }
    bool isValid() const { return it->isValid(); }
    
    void reset() { it->reset(); }
    
  private:
    const RIFG& rifg;
    OA_ptr<OA::DGraph::EdgesIteratorInterface> it;
  };


  class OutgoingEdgesIterator {
  public:
    OutgoingEdgesIterator(const RIFG& rifg_, RIFG::NodeId nid) 
      : rifg(rifg_) 
    {
      OA::OA_ptr<OA::DGraph::NodeInterface> n = rifg.getNode(nid);
      it = n->getOutgoingEdgesIterator();
    }
    ~OutgoingEdgesIterator() { }
    
    RIFG::EdgeId current() const {
      OA::OA_ptr<OA::DGraph::EdgeInterface> edge = it->current();
      return rifg.getEdgeId(edge);
    }
    void operator++() { ++(*it); }
    bool isValid() const { return it->isValid(); }
    
    void reset() { it->reset(); }
    
  private:
    const RIFG& rifg;
    OA_ptr<OA::DGraph::EdgesIteratorInterface> it;
  };

  
public:
  RIFG(OA_ptr<DGraph::DGraphInterface> graph, 
       OA_ptr<DGraph::NodeInterface> source,
       OA_ptr<DGraph::NodeInterface> sink);
  ~RIFG();
  
  OA_ptr<DGraph::DGraphInterface> getGraph() { return graph; }
  
  // largest node/edge id in the graph
  NodeId getHighWaterMarkNodeId() const { return highWaterMarkNodeId; }
  EdgeId getHighWaterMarkEdgeId() const { return highWaterMarkEdgeId; }

  // is the node id still valid, or has it been freed
  bool isValid(NodeId nin) const;

  // Map between nodes/edges and node-ids/edge-ids.  Ids can be tested
  // against RIFG::NULL for validity.
  OA::OA_ptr<OA::DGraph::NodeInterface> 
  getNode(const NodeId nid) const
  { 
    OA::OA_ptr<OA::DGraph::NodeInterface> n;
    IdToNodeMap_t::const_iterator it = id_to_node_map.find(nid);
    if (it != id_to_node_map.end()) { n = (*it).second; }
    return n;
  }
  OA::OA_ptr<OA::DGraph::EdgeInterface> 
  getEdge(const EdgeId eid) const
  { 
    OA::OA_ptr<OA::DGraph::EdgeInterface> e;
    IdToEdgeMap_t::const_iterator it = id_to_edge_map.find(eid);
    if (it != id_to_edge_map.end()) { e = (*it).second; }
    return e;
  }

  NodeId 
  getNodeId(const OA::OA_ptr<OA::DGraph::NodeInterface> n) const
  { 
    NodeToIdMap_t::const_iterator it = node_to_id_map.find(n);
    return (it != node_to_id_map.end()) ? (*it).second : RIFG::NIL;
  }
  EdgeId 
  getEdgeId(const OA::OA_ptr<OA::DGraph::EdgeInterface> e) const
  { 
    EdgeToIdMap_t::const_iterator it = edge_to_id_map.find(e);
    return (it != edge_to_id_map.end()) ? (*it).second : RIFG::NIL;
  }

  // graph methods operating on the node-id/edge-id abstraction
  NodeId getSource() const;
  NodeId getSink() const;
  
  NodeId getEdgeSrc(EdgeId eid)  const;
  NodeId getEdgeSink(EdgeId eid) const;
  
  OA_ptr<NodesIterator> getNodesIterator() const;
  OA_ptr<IncomingEdgesIterator> getIncomingEdgesIterator(NodeId nid) const;
  OA_ptr<OutgoingEdgesIterator> getOutgoingEdgesIterator(NodeId nid) const;
  
  void dumpNode(std::ostream& os, NodeId nid);


  // helper methods to find the source and sink of a DGraph.  These
  // may be used with the constructor, e.g.:
  //  RIFG(graph, RIFG::getEntryNode(graph), RIFG::getExitNode(graph))

  static OA_ptr<DGraph::NodeInterface>
    getSourceNode(OA_ptr<DGraph::DGraphInterface> graph);

  static OA_ptr<DGraph::NodeInterface> 
    getSinkNode(OA_ptr<DGraph::DGraphInterface> graph);


  friend class NodesIterator;

private:
  OA_ptr<DGraph::DGraphInterface> graph;
  OA_ptr<DGraph::NodeInterface> mSource; // graph source ('entry')
  OA_ptr<DGraph::NodeInterface> mSink;   // graph sink ('exit')

  NodeId highWaterMarkNodeId;
  EdgeId highWaterMarkEdgeId;
  
  IdToNodeMap_t id_to_node_map;
  NodeToIdMap_t node_to_id_map;
  IdToEdgeMap_t id_to_edge_map;
  EdgeToIdMap_t edge_to_id_map;

};

} // end of namespace OA

#endif
