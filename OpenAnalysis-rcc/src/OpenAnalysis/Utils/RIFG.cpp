/*! \file
  
  \brief Representation Independent Flowgraph Interface.  This is a
         wrapper that allows access to an OA DGraph through dense id
         numbers ranging from 0 to n.  It is useful to interface with
         algorithms that worked by creating a work or result array of
         size n.  It might more appropriately be a mixin to an OA
         DGraph.
        
  \authors Original DSystem code by John Mellor-Crummey
           Port from 'Classic OpenAnalysis' to OpenAnalysis by Nathan
             Tallent (merged RIFG and OARIFG)
  \version $Id: RIFG.cpp,v 1.5 2005/06/10 02:32:05 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


//************************** System Include Files ***************************

#ifdef NO_STD_CHEADERS
# include <assert.h>
# include <limits.h>
#else
# include <cassert>
# include <climits>
#endif

#include <inttypes.h>

//*************************** User Include Files ****************************

#include <OpenAnalysis/Utils/RIFG.hpp>
#include <OpenAnalysis/CFG/CFGInterface.hpp>

//*************************** Forward Declarations **************************

//***************************************************************************

//***************************************************************************
// Representation Independent Flowgraph
//***************************************************************************

namespace OA {

unsigned int RIFG::NIL = 0;

RIFG::RIFG(OA_ptr<DGraph::DGraphInterface> graph_, 
	   OA_ptr<DGraph::NodeInterface> source,
	   OA_ptr<DGraph::NodeInterface> sink)
  : graph(graph_), mSource(source), mSink(sink)
{
  // Sanity check
  assert(graph->getNumNodes() <= UINT_MAX);
  assert(graph->getNumEdges() <= UINT_MAX);
  
  // Initialize the node-id to node map: ids are 1...num_nodes
  highWaterMarkNodeId = graph->getNumNodes();
  OA::OA_ptr<OA::DGraph::NodesIteratorInterface> it1 = 
    graph->getNodesIterator();
  for (NodeId i = 1; it1->isValid(); ++(*it1), ++i) {
    OA::OA_ptr<OA::DGraph::NodeInterface> node = it1->current();
    node_to_id_map[node] = i;
    id_to_node_map[i] = node;
  }
  
  // Initialize the edge-id to edge map: ids are 1...num_edges
  highWaterMarkEdgeId = graph->getNumEdges();
  OA::OA_ptr<OA::DGraph::EdgesIteratorInterface> it2 = 
    graph->getEdgesIterator();
  for (NodeId i = 1; it2->isValid(); ++(*it2), ++i) {
    OA::OA_ptr<OA::DGraph::EdgeInterface> edge = it2->current();
    edge_to_id_map[edge] = i;
    id_to_edge_map[i] = edge;
  }
}


RIFG::~RIFG()
{
  node_to_id_map.clear();
  id_to_node_map.clear();
  edge_to_id_map.clear();
  id_to_edge_map.clear();
}


bool 
RIFG::isValid(RIFG::NodeId nid) const
{
  return true;
}


RIFG::NodeId 
RIFG::getSource() const
{
  return (getNodeId(mSource));
}


RIFG::NodeId 
RIFG::getSink() const
{
  return (getNodeId(mSink));
}

RIFG::NodeId 
RIFG::getEdgeSrc(RIFG::EdgeId eid) const
{
  OA::OA_ptr<OA::DGraph::EdgeInterface> e = getEdge(eid);
  OA::OA_ptr<OA::DGraph::NodeInterface> n = e->getSource();
  return (getNodeId(n));
}


RIFG::NodeId 
RIFG::getEdgeSink(RIFG::EdgeId eid) const
{
  OA::OA_ptr<OA::DGraph::EdgeInterface> e = getEdge(eid);
  OA::OA_ptr<OA::DGraph::NodeInterface> n = e->getSink();
  return (getNodeId(n));
}


OA_ptr<RIFG::NodesIterator>
RIFG::getNodesIterator() const
{
  OA_ptr<RIFG::NodesIterator> it;
  it = new NodesIterator(*this);
  return it;
}


OA_ptr<RIFG::IncomingEdgesIterator>
RIFG::getIncomingEdgesIterator(NodeId nid) const
{
  OA_ptr<RIFG::IncomingEdgesIterator> it;
  it = new IncomingEdgesIterator(*this, nid);
  return it;
}


OA_ptr<RIFG::OutgoingEdgesIterator>
RIFG::getOutgoingEdgesIterator(NodeId nid) const
{
  OA_ptr<RIFG::OutgoingEdgesIterator> it;
  it = new OutgoingEdgesIterator(*this, nid);
  return it;
}


void 
RIFG::dumpNode(std::ostream& os, RIFG::NodeId nid)
{
  // getNode(nid)->dump(os);
}


//***************************************************************************
// 
//***************************************************************************

OA_ptr<DGraph::NodeInterface> 
RIFG::getSourceNode(OA_ptr<DGraph::DGraphInterface> graph)
{
  if (graph.isa<OA::CFG::CFGInterface>()) {
    OA_ptr<OA::CFG::CFGInterface> g = graph.convert<OA::CFG::CFGInterface>();
    OA_ptr<OA::CFG::NodeInterface> n = g->getEntry();
    return n;
  }
  else {
    OA_ptr<DGraph::NodesIteratorInterface> it = 
      graph->getEntryNodesIterator();
    assert(it->isValid());
    OA_ptr<DGraph::NodeInterface> n = it->current();
    return n;
  }
}

OA_ptr<DGraph::NodeInterface> 
RIFG::getSinkNode(OA_ptr<DGraph::DGraphInterface> graph)
{
  if (graph.isa<OA::CFG::CFGInterface>()) {
    OA_ptr<OA::CFG::CFGInterface> g = graph.convert<OA::CFG::CFGInterface>();
    OA_ptr<OA::CFG::NodeInterface> n = g->getExit();
    return n;
  }
  else {
    OA_ptr<DGraph::NodesIteratorInterface> it = 
      graph->getExitNodesIterator();
    assert(it->isValid());
    OA_ptr<DGraph::NodeInterface> n = it->current();
    return n;
  }
}

} // end of namespace OA

