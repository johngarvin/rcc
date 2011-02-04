/*! \file
  
  \brief Test driver for directed graph class

  \authors Arun Chauhan (2001 was part of Mint), Michelle Strout
  \version $Id: grtest.cpp,v 1.4 2004/11/19 19:21:53 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include <iostream>
#include <OpenAnalysis/Utils/DGraph/DGraphStandard.hpp>

/*
using std::endl;
using std::cout;
using std::cerr;
*/
using namespace std;
//using namespace OA;


class myGraph : public OA::DGraph::DGraphStandard {
public:
  class Node : public OA::DGraph::DGraphStandard::Node {
  public:
    Node (int v) : OA::DGraph::DGraphStandard::Node() { val = v; }
    int val;
  };
  class Edge : public OA::DGraph::DGraphStandard::Edge {
  public:
    Edge (Node* src, Node* sink, int v) : OA::DGraph::DGraphStandard::Edge(src, sink) { val = v; }
    int val;
  };

  myGraph (Node* root) : OA::DGraph::DGraphStandard (root) {}

  //int edge_count () { return edge_set.size(); }
  //int node_count () { return node_set.size(); }
  int edge_count () { return -1; }
  int node_count () { return -1; }
};


void list_DFS_nodes (myGraph&);
void list_ReversePostDFS_nodes (myGraph&);
void dump_graph (myGraph&);

int
main (int argc, char* argv[])
{
  try {
    // must allocate on heap because the graph datastructures
    // manipulate Edge and Node ptrs, including calling delete
    myGraph::Node *n1 = new myGraph::Node(1);
    myGraph::Node *n2 = new myGraph::Node(2);
    myGraph::Node *n3 = new myGraph::Node(3);
    myGraph::Node *n4 = new myGraph::Node(4);
    myGraph::Edge *e1 = new myGraph::Edge(n1, n2, 1);
    myGraph::Edge *e2 = new myGraph::Edge(n1, n3, 1);
    myGraph::Edge *e3 = new myGraph::Edge(n2, n4, 1);
    myGraph::Edge *e4 = new myGraph::Edge(n3, n4, 1);

    myGraph g(n1);

    g.add(e1);
    g.add(e2);
    g.add(e3);
    g.add(e4);

    dump_graph(g);
    list_DFS_nodes(g);
    list_ReversePostDFS_nodes(g);
//    cout << "Initial graph" << endl;
//    dump_graph(g);
    g.remove(n2);
    cout << "After deleting node 2" << endl;
//    //list_DFS_nodes(g);
    dump_graph(g);

    cout << "Graph 2" << endl;
    myGraph::Node *nn = new myGraph::Node(5);
    //  myGraph2::node nn2(1.3);
    myGraph g2(nn);
    cout << "number of edges in g = " << g.edge_count() << ", number of edges in g2 = " << g2.edge_count() << endl;
    cout << "number of nodes in g = " << g.node_count() << ", number of nodes in g2 = " << g2.node_count() << endl;

    //  g2.add_node(&nn2);
    //  g2.add_edge(new myGraph2::edge(&nn, &nn2, 2.5));
    //list_DFS_nodes(g2);
    dump_graph(g2);

    //    g2.add(&e4);
    g2.add(n3);

  }
  catch (OA::Exception& e) {
    e.report(cerr);
  }

}


void
list_DFS_nodes (myGraph& g)
{
  cout << "Enumerating Nodes in DFS order:" << endl;
  myGraph::DFSIterator iter(g);
  while ((bool)iter) {
    myGraph::Node* n = dynamic_cast<myGraph::Node*>(iter.current());
    cout << "\tnode " << n->val << endl;
    ++iter;
  }
}

void
list_ReversePostDFS_nodes (myGraph& g)
{

  cout << "Enumerating Nodes in ReversePostDFS order using local iter:" << endl;
  OA::DGraph::DGraphStandard::ReversePostDFSIterator iter(g,OA::DGraph::DEdgeOrg);
  while ((bool)iter) {
    myGraph::Node* n = dynamic_cast<myGraph::Node*>(iter.current());
    cout << "\tnode " << n->val << endl;
    ++iter;
  }


  cout << "Enumerating Nodes in ReversePostDFS order getReversePostDFSIterator:" << endl;
  OA::DGraph::Interface::ReversePostDFSIterator * iterPtr = g.getReversePostDFSIterator(OA::DGraph::DEdgeOrg);
  while ((bool)(*iterPtr)) {
    myGraph::Node* n = dynamic_cast<myGraph::Node*>(iterPtr->current());
    cout << "\tnode " << n->val << endl;
    ++(*iterPtr);
  }
}

void
dump_graph (myGraph& g)
{
  cout << "Enumerating all Nodes in arbitrary order:" << endl;
  myGraph::NodesIterator n_iter(g);
  while ((bool)n_iter) {
    //myGraph::Node* n = dynamic_cast<myGraph::Node*>((OA::DGraph::DGraphStandard::Node*)n_iter);
    myGraph::Node* n = dynamic_cast<myGraph::Node*>(n_iter.current());
    cout << "\tnode " << n << " = " << n->val << endl;
    ++n_iter;
  }
  cout << "Enumerating all Edges in arbitrary order:" << endl;
  myGraph::EdgesIterator e_iter(g);
  while ((bool)e_iter) {
    //myGraph::Edge* e = dynamic_cast<myGraph::Edge*>((OA::DGraph::DGraphStandard::Edge*)e_iter);
    myGraph::Edge* e = dynamic_cast<myGraph::Edge*>(e_iter.current());
    cout << "\tedge " << e << " = " << e->val << endl;
    ++e_iter;
  }
}
