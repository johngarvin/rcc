/*! \file
  
  \brief Definition for Graph class

  \authors Arun Chauhan (2001 was part of Mint)
  \version $Id: Graph.cpp,v 1.11 2005/03/18 18:45:55 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>


  Graph is the base class (the "interface") for a general undirected
  graph (Graph).  Algorithms that operate upon abstract undirected
  graphs should, normally, use only this base Graph class for maximum
  portability.
*/


#include "Graph.hpp"

namespace OA {

//--------------------------------------------------------------------
/*! This routine must be called before a DFS iterator can be used.  It
    traverses the graph in DFS order and adds, in
    each node, a pointer to the next node in DFS ordering. */
OA_ptr<BaseGraph::Node>
Graph::create_DFS_links (OA_ptr<BaseGraph::Node> n)
{/* AIS - commented out in order to get OA to compile
  OA_ptr<NodesIterator> neigh =
    (n.convert<Node>())->getNeighborNodesIterator();;
  OA_ptr<BaseGraph::Node> last = n;
  while (neigh->isValid()) {
    if (neigh->current()->dfs_succ.ptrEqual(0)) {
      OA_ptr<Node> n = neigh->current(); // Stupid Sun CC 5.4
      // cast is needed to access protected member
      (last.convert<Node>())->dfs_succ = n.convert<BaseGraph::Node>();
      last = create_DFS_links(neigh->current());
    }
    ++(*neigh);
  }
  // cast is needed to access protected member
  (last.convert<Node>())->dfs_succ = 0;
  return last;*/
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! This routine must be called before a BFS iterator can be used.  It
    traverses the graph in BFS order and adds, in
    each node, a pointer to the next node in BFS ordering. */
OA_ptr<BaseGraph::Node>
Graph::create_BFS_links (OA_ptr<BaseGraph::Node> n)
{
  return n;
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! Add the given node to the set of nodes. */
void
Graph::addNode (OA_ptr<Graph::Node> n)
{
  BaseGraph::addNode(n);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! Add the edge to the set of edges, as well as to the list of
    incident edges in both the nodes involved.  Further,
    add the two nodes to each other's neighbor lists. */
void
Graph::addEdge (OA_ptr<Graph::Edge> e)
{
  BaseGraph::addEdge(e);
  e->node1()->incident_edges->push_back(e);
  e->node2()->incident_edges->push_back(e);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! Remove this edge from the BaseGraph as well as from the list of
    incident edges of the two nodes that form this
    edge. */
void
Graph::removeEdge (OA_ptr<Graph::Edge> e)
{
  BaseGraph::removeEdge(e);

  // remove this edge from the list of incident edges of the two nodes involved
  e->node1()->incident_edges->remove(e);
  e->node2()->incident_edges->remove(e);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*! Remove the given node and all the edges incident on it. */
void
Graph::removeNode (OA_ptr<Graph::Node> n)
{
/* AIS - temporarily commented out so I can get OA to compile.
  BaseGraph::removeNode(n);

  // remove all the edges incident on this node
  OA_ptr<EdgesIterator> incident = n->getIncidentEdgesIterator();
  while (incident->isValid()) {
    OA_ptr<Edge> e = incident->current();
    // remove this edge from the neighboring node's list of incident edges
    if (e->node1().ptrEqual(n)) {
      e->node2()->incident_edges->remove(e);
    } else {
      e->node1()->incident_edges->remove(e);
    }
    // remove this edge
    BaseGraph::removeEdge(e);
    ++incident;
  }*/
}
//--------------------------------------------------------------------

} // end of OA namespace
