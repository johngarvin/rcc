/*! \file
  
  \brief Declaration for standard CFGStandard.

  \authors Arun Chauhan (2001 as part of Mint)
           Port from 'Classic OpenAnalysis' to OpenAnalysis by Nathan Tallent
  \version $Id: DomTree.cpp,v 1.5 2005/04/16 17:23:18 eraxxon Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


//---------------------------------------------------------------------------

// OpenAnalysis headers
#include "DomTree.hpp"

#include <iostream>
using std::ostream;
using std::endl;

// STL headers
#include <algorithm>
#include <vector>

//---------------------------------------------------------------------------

namespace OA {

//***************************************************************************
// 
//***************************************************************************

/** Construct the dominator tree for the given directed graph.  This
    is (currently) done in the most straight-forward manner by
    iteratively solving data-flow equations for dominator sets.  While
    this dataflow framework is guaranteed rapid convergence, perhaps,
    a better (but more complicated) algorithm to implement would be
    Lengauer-Tarjan's that appears in "A Fast Algorithm for Finding
    Dominators in a Flowgraph", ACM Transactions on Programming
    Languages and Systems, Vol. 1, No. 1, July 1979, pages 121-141. 
    
    FIXME: Use RIFG instead of computing node ids in first step
*/

DomTree::DomTree(OA_ptr<DGraph::DGraphInterface> graph_,
		 OA_ptr<DGraph::NodeInterface> root_)
  : Tree(), graph(graph_)
{
  int i;

  // first, populate the tree with nodes corresponding to the directed
  // graph g and number them for easy reference
  int S = graph->getNumNodes();
  std::vector<OA_ptr<Node> > tree_node_vec(S);
  std::vector<OA_ptr<DGraph::NodeInterface> > graph_node_vec(S);
  std::vector<std::set<int> > dom_set(S);
  std::map<OA_ptr<DGraph::NodeInterface>, int> graph_node_num;
  
  int count = 0;
  OA_ptr<DGraph::NodesIteratorInterface> dfs_iter = graph->getDFSIterator(root_);
  for ( ; dfs_iter->isValid(); ++(*dfs_iter)) {
    OA_ptr<DGraph::NodeInterface> n = dfs_iter->current();
    tree_node_vec[count] = new Node(n);
    graph_node_vec[count] = n;
    graph_node_num[n] = count;
    dom_tree_node[n] = tree_node_vec[count];
    ++count;
  }
  addNode(tree_node_vec[0]); // add the root node
  dom_set[0].insert(1); // initialize the root node's dom set
  
  // now propagate the dominator sets for each node iteratively until
  // a fixed point is reached but, we do this only for nodes that are
  // actually reachable from the root ("count" nodes)
  bool changed = true;
  while (changed) {
    changed = false;
    std::set<int> tmp_dom;
    for (i=0; i < count; i++) {
      // intersect the dom sets of each of the predecessors
      tmp_dom.clear();

      OA_ptr<DGraph::NodesIteratorInterface> src_iter = 
	graph_node_vec[i]->getSourceNodesIterator();
      while (src_iter->isValid() && 
	     dom_set[graph_node_num[src_iter->current()]].empty())
	++(*src_iter);
      if (src_iter->isValid()) {
	int node_num = graph_node_num[src_iter->current()];
	tmp_dom = dom_set[node_num];
	++(*src_iter);
      }
      while (src_iter->isValid()) {
	int node_num = graph_node_num[src_iter->current()];
	if (!dom_set[node_num].empty()) {
	  std::set<int> tmp;
	  std::set_intersection(tmp_dom.begin(), tmp_dom.end(), 
				dom_set[node_num].begin(), 
				dom_set[node_num].end(),
				std::inserter(tmp, tmp.begin()));
	  tmp_dom = tmp;
	}
	++(*src_iter);
      }
      tmp_dom.insert(i);
      if (!(tmp_dom == dom_set[i])) {
	changed = true;
	dom_set[i] = tmp_dom;
      }
    }
  }

  // finally, find the immediate dominator of each node and insert
  // appropriate edges
  for (i=0; i < count; i++) {
    std::set<int>::iterator ds_iter = dom_set[i].begin();
    int parent_of_i = -1;
    while (ds_iter != dom_set[i].end()) {
      int val = *ds_iter;
      if ((parent_of_i < val) && (val != i))
	parent_of_i = val;
      ++ds_iter;
    }
    if (parent_of_i >= 0) {
      OA_ptr<Edge> e; 
      e = new Edge(tree_node_vec[parent_of_i], tree_node_vec[i]);
      addEdge(e);
    }
  }

}


/** Dominance frontier for the dominator tree is computed simply by
    looking at each branch point in the directed graph (a node that
    has more than one predecessor) and adding such a node to the
    dominance frontier of the chain of immediate dominators of each of
    its predecessors terminating at the node's immediate dominator.
    The function uses the following algorithm, from the book
    "Engineering a Compiler", by Keith D. Cooper and Linda Torczon,
    chapter "Data-flow Analysis".

    <PRE>
    for all nodes b
      if the number of predecessors of b > 1
          for all predecessors, p, of b
              runner <-- p
                 while runner != IMMEDIATE_DOMINATOR(b)
                       add b to DOMINANCE_FRONTIER(runner)
                       runner = IMMEDIATE_DOMINATOR(runner)
    </PRE>
*/
void
DomTree::compute_dominance_frontiers ()
{
  OA_ptr<DGraph::NodesIteratorInterface> nodes_iter = 
    graph->getNodesIterator();
  while (nodes_iter->isValid()) {
    OA_ptr<DGraph::NodeInterface> b = nodes_iter->current();
    if (b->num_incoming() > 1) {
      OA_ptr<DGraph::NodesIteratorInterface> p = 
	b->getSourceNodesIterator();
      while (p->isValid()) {
	OA_ptr<DGraph::NodeInterface> runner = p->current();
	// this parent may be unreachable in the control flow graph and, hence, may have no corresponding Dominator
	// Tree node
	if (dom_tree_node.find(runner) != dom_tree_node.end()) {
	  OA_ptr<Node> b_dom_tree_node = dom_tree_node[b];
	  OA_ptr<Node> runner_dom_tree_node = dom_tree_node[runner];
	  while (runner_dom_tree_node != b_dom_tree_node->parent()) {
	    if (runner_dom_tree_node->dom_front.find(b_dom_tree_node) == 
		runner_dom_tree_node->dom_front.end()) {
	      runner_dom_tree_node->dom_front.insert(b_dom_tree_node);
	    }
	    OA_ptr<Node> n = runner_dom_tree_node->parent();
	    runner = n->graph_node_ptr;
	    runner_dom_tree_node = dom_tree_node[runner];
	  }
	}
	++(*p);
      }
    }
    ++(*nodes_iter);
  }
}


void
DomTree::dump (ostream& os)
{
  OA_ptr<NodesIterator> nodes_iter = getNodesIterator();
  while (nodes_iter->isValid()) {
    OA_ptr<Tree::Node> treenode = nodes_iter->current();
    OA_ptr<Node> n = treenode.convert<Node>();
    os << "[";
    if (!n->parent().ptrEqual(0)) {
      OA_ptr<Node> n1 = n->parent();
      n1->graph_node_ptr->dump(os);
    }
    else {
      os << "root";
    }
    os << "] --> ";
    n->graph_node_ptr->dump(os);
    os << " --> {";
    OA_ptr<ChildNodesIterator> child_iter = n->getChildNodesIterator();
    if (child_iter->isValid()) {
      OA_ptr<Tree::Node> n = child_iter->current();
      OA_ptr<Node> n1 = n.convert<Node>();
      n1->graph_node_ptr->dump(os);
      ++(*child_iter);
      for ( ; child_iter->isValid(); ++(*child_iter)) {
        OA_ptr<Tree::Node> n = child_iter->current();
        OA_ptr<Node> n1 = n.convert<Node>();
        os << ", ";
        n1->graph_node_ptr->dump(os);
      }
    }
    os << "}  DF = {";
    OA_ptr<DomFrontIterator> front_iter = n->getDomFrontIterator();
    if (front_iter->isValid()) {
      front_iter->current()->dump(os);
      ++(*front_iter);
      for ( ; front_iter->isValid(); ++(*front_iter)) {
	os << ", ";
	front_iter->current()->dump(os);
      }
    }
    os << "}" << endl;
    ++(*nodes_iter);
  }
}


//***************************************************************************
// 
//***************************************************************************

void 
DomTree::Node::dump(std::ostream& os)
{
  OA_ptr<DGraph::NodeInterface> n = getGraphNode();
  n->dump(os);
  
  DomFrontSet::iterator it = dom_front.begin();
  for ( ; it != dom_front.end(); ++it) {
    OA_ptr<Node> n = *it;
    os << "<treenode> ";
  }
}


} // end of namespace OA

